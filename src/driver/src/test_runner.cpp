#include <driver/test_runner.hpp>
#include <driver/wave_scheduler.hpp>
#include <iomodels/iomanager.hpp>
#include <boost/lexical_cast.hpp>
#include <filesystem>
#include <iostream>

test_runner::test_runner(program_options_ptr options)
    : options_(std::move(options))
{
}

void test_runner::run()
{
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

    auto time_limit = boost::lexical_cast<size_t>(options_->value("max_exec_milliseconds"));
    auto waves = calculate_waves(time_limit, tests.size());

    execute_waves(*executor, tests, *analyzer, waves);

    auto results = collect_results(*analyzer);
    report_results(results, test_type);
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
    if (std::filesystem::is_directory(output_dir_ / "progress_recording"))
        std::filesystem::remove_all(output_dir_ / "progress_recording");
}

void test_runner::configure_io_limits() const
{
    iomodels::iomanager::instance().set_config(
        {.max_exec_milliseconds = (natural_16_bit)std::max(
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

void test_runner::execute_waves(connection::target_executor& executor,
                                 TestDirParser::tests& tests,
                                 IRunAnalyzer& analyzer,
                                 const std::vector<size_t>& waves)
{
    for (auto wave_timeout : waves) {
        executor.set_timeout(wave_timeout);
        std::cout << "Running test with timeout: " << wave_timeout << std::endl;

        for (auto it = tests.begin(); it != tests.end();) {
            write_test_input(executor.get_shared_memory(), *it);

            executor.execute_target();

            analyzer.add_execution(executor.get_shared_memory());

            auto termination = executor.get_shared_memory().get_termination();

            if (termination != instrumentation::target_termination::normal) {
                std::cout << "Abnormal termination: " << (int)*termination << std::endl;
            }

            if (termination == instrumentation::target_termination::timeout) {
                ++it;
            } else {
                tests.erase(it++);
            }
        }
    }
}

void test_runner::write_test_input(connection::shared_memory& shm,
                                    const TestBuffer& test)
{
    uint64_t byte_count = test.byte_count();

    shm.clear();
    shm << (uint16_t)max_exec_megabytes_;
    shm << byte_count;
    shm << test.input_count();
    shm.accept_bytes(test.data(), byte_count);
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
