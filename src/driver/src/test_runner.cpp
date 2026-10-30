#include <driver/test_runner.hpp>
#include <driver/wave_scheduler.hpp>
#include <iomodels/iomanager.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>

test_runner::test_runner(program_options_ptr options)
    : options_(std::move(options))
{
}

void test_runner::run()
{
    auto time_limit = boost::lexical_cast<size_t>(options_->value("max_exec_milliseconds"));
    time_budget budget(time_limit);

    validate_options();
    prepare_output_directory();
    configure_io_limits();

    const auto executor = std::make_shared<connection::target_executor>(
        options_->value("path_to_target"));

    auto tests = load_test_suite();

    TestType test_type = determine_test_type();

    auto analyzer = create_run_analyzer(test_type);

    std::cout << "Test count: " << tests.size() << "\nTest type: " << options_->value("goal") << std::endl;

    max_exec_megabytes_ = boost::lexical_cast<size_t>(options_->value("max_exec_megabytes"));
    executor->init_shared_memory(1024 * 1024 * max_exec_megabytes_);

    auto summary = execute_waves(*executor, tests, *analyzer, budget);

    auto results = collect_results(*analyzer);
    results.time_limit_reached = summary.time_limit_reached;
    results.elapsed_ms = budget.elapsed_ms();
    results.tests_executed = summary.executed;
    results.tests_skipped = summary.skipped;
    results.stop_reason = summary.reason == execution_stop_reason::completed
        ? "completed"
        : summary.reason == execution_stop_reason::time_budget_depleted
            ? "time_budget_depleted"
            : "budget_too_small";
    report_results(results, test_type);
    report_time_budget(budget, summary);
}

void test_runner::validate_options() const
{
    if (options_->value("output_dir").empty()) {
        std::cerr << "ERROR: The output directory path is empty.\n";
        std::exit(1);
    }

    if (!options_->has("path_to_target")) {
        std::cerr << "ERROR: The path to target is empty.\n";
        std::exit(1);
    }
    if (!std::filesystem::is_regular_file(options_->value("path_to_target"))) {
        std::cerr << "ERROR: The passed target path '"
                  << options_->value("path_to_target")
                  << "' does not reference a regular file.\n";
        std::exit(1);
    }
    std::filesystem::perms const perms =
        std::filesystem::status(options_->value("path_to_target")).permissions();
    if ((perms & std::filesystem::perms::owner_exec) == std::filesystem::perms::none) {
        std::cerr << "ERROR: The passed target path '"
                  << options_->value("path_to_target")
                  << "' references a file which is NOT executable.\n";
        std::exit(1);
    }

    if (options_->has("path_to_client")) {
        if (!std::filesystem::is_regular_file(options_->value("path_to_client"))) {
            std::cerr << "ERROR: The passed client path '"
                      << options_->value("path_to_client")
                      << "' does not reference a regular file.\n";
            std::exit(1);
        }
        std::filesystem::perms const perms2 =
            std::filesystem::status(options_->value("path_to_client")).permissions();
        if ((perms2 & std::filesystem::perms::owner_exec) == std::filesystem::perms::none) {
            std::cerr << "ERROR: The passed client path '"
                      << options_->value("path_to_client")
                      << "' references a file which is NOT executable.\n";
            std::exit(1);
        }
    }
}

void test_runner::prepare_output_directory()
{
    output_dir_ = std::filesystem::absolute(options_->value("output_dir"));

    std::error_code ec;
    std::filesystem::create_directories(output_dir_, ec);
    if (ec) {
        std::cerr << "ERROR: Failed to create/access the output directory:\n        "
                  << output_dir_ << "\n";
        std::exit(1);
    }

    if (!options_->has("clear_output_dir"))
        return;

    for (const auto& entry : std::filesystem::directory_iterator(output_dir_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            std::filesystem::remove(entry);
    }
    if (std::filesystem::is_directory(output_dir_ / "test-suite")) {
        for (const auto& entry : std::filesystem::directory_iterator(output_dir_ / "test-suite"))
            std::filesystem::remove(entry);
    }
}

void test_runner::configure_io_limits() const
{
    iomodels::iomanager::instance().set_config(
        {.max_exec_milliseconds = (natural_32_bit)std::max(
             0,
             std::stoi(options_->value("max_exec_milliseconds"))),
         .max_exec_megabytes = (natural_16_bit)std::max(
             0,
             std::stoi(options_->value("max_exec_megabytes")))});
}

TestType test_runner::determine_test_type() const
{
    TestType test_type = options_->value("goal") == "coverage" ? BRANCH_COVERAGE : ERROR_CALL;
    if (test_type == BRANCH_COVERAGE && options_->has("testcomp")) {
        std::cout << "TestComp mode" << std::endl;
        test_type = TESTCOMP_COVERAGE;
    }
    return test_type;
}

TestDirParser::tests test_runner::load_test_suite() const
{
    return TestDirParser::parse_dir(options_->value("test_dir"));
}

test_runner::execution_summary test_runner::execute_waves(
    connection::target_executor& executor,
    TestDirParser::tests& tests,
    IRunAnalyzer& analyzer,
    const time_budget& budget)
{
    execution_summary summary;
    std::size_t wave_index = 0;

    while (!tests.empty() && wave_index < wave_safety_max) {
        const auto plan = next_wave_plan(budget, wave_index, tests.size());

        if (!plan) {
            summary.time_limit_reached = true;
            summary.reason = budget.usable_ms() < min_useful_slice_ms
                ? execution_stop_reason::time_budget_depleted
                : execution_stop_reason::budget_too_small;
            break;
        }

        ++summary.waves;
        std::cout << "Running wave " << plan->wave_index
                  << " with timeout: " << plan->timeout_ms << " ms" << std::endl;

        bool budget_depleted = false;

        for (auto it = tests.begin(); it != tests.end();) {
            const std::size_t usable = budget.usable_ms();

            if (usable < min_useful_slice_ms) {
                budget_depleted = true;
                break;
            }

            executor.set_timeout(std::min(plan->timeout_ms, usable));
            write_test_input(executor.get_shared_memory(), *it);

            const auto execution =
                executor.execute_target(it->command_line_args, it->stdin_data);

            if (execution.started && execution.header_valid)
                analyzer.add_execution(executor.get_shared_memory());

            ++summary.executed;

            if (execution.timed_out) {
                ++summary.retried;
                ++it;
            } else {
                tests.erase(it++);
            }
        }

        if (budget_depleted) {
            summary.time_limit_reached = true;
            summary.reason = execution_stop_reason::time_budget_depleted;
            break;
        }

        ++wave_index;
    }

    summary.skipped = tests.size();

    if (!tests.empty()) {
        summary.time_limit_reached = true;
        if (summary.reason == execution_stop_reason::completed)
            summary.reason = execution_stop_reason::time_budget_depleted;
    }

    return summary;
}

void test_runner::write_test_input(connection::shared_memory& shm,
                                    const TestDirParser::test_case& test)
{
    const auto& input = test.input;
    uint64_t byte_count = input.byte_count();

    shm.reset_for_test();
    shm << (uint16_t)max_exec_megabytes_;
    shm << byte_count;
    shm << input.input_count();
    shm.accept_bytes(input.data(), byte_count);
}

test_result test_runner::collect_results(IRunAnalyzer& analyzer) const
{
    double score = analyzer.get_result();

    assert(score >= 0.0 && score <= 1.0);

    return {score, analyzer.get_coverage_map()};
}

void test_runner::report_results(const test_result& result, TestType test_type) const
{
    std::cout << "Coverage: " << std::setprecision(4) << result.score << std::endl;

    if (test_type == ERROR_CALL) {
        std::cout << "Result: " << (result.score ? "TRUE" : "FALSE") << std::endl;
    } else {
        std::cout << "Result: DONE" << std::endl;
    }

    auto result_file = output_dir_ / "result.json";
    write_result_json(result_file, result);
}

void test_runner::report_time_budget(const time_budget& budget,
                                     const execution_summary& summary) const
{
    const char* reason = "completed";
    if (summary.reason == execution_stop_reason::time_budget_depleted)
        reason = "time_budget_depleted";
    else if (summary.reason == execution_stop_reason::budget_too_small)
        reason = "budget_too_small";

    std::cout << "Time budget: " << budget.total_ms() << " ms\n"
              << "Elapsed: " << budget.elapsed_ms() << " ms\n"
              << "Waves: " << summary.waves << "\n"
              << "Executed: " << summary.executed
              << ", retried: " << summary.retried
              << ", skipped: " << summary.skipped << "\n"
              << "Stop reason: " << reason << std::endl;
}
