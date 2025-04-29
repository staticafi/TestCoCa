#include <boost/process/exe.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <connection/target_executor.hpp>
#include <driver/program_options.hpp>
#include <filesystem>
#include <iomodels/iomanager.hpp>
#include <iostream>
#include <boost/math/special_functions/pow.hpp>
#include <boost/lexical_cast.hpp>

#include "driver/run_analyzer.hpp"
#include "driver/test_parser.hpp"

#define TEST_MIN_DURATION 100
#define TEST_MAX_WAVE_COUNT 4
#define TEST_DURATION_INCREASE_RATIO 5

void save_results_to_json(const std::string& filename, const std::pair<double, coverage_map>& results)
{
    boost::property_tree::ptree root;

    float coverage = std::round(results.first * 10000.0f) / 10000.0f;

    root.put("coverage", coverage);

    boost::property_tree::ptree coverage_node;
    for (const auto& [key, value] : results.second) {
        std::string key_str = std::to_string(key);
        switch (value) {
            case instrumentation::FALSE:
                coverage_node.put<std::string>(key_str, "FALSE");
                break;
            case instrumentation::TRUE:
                coverage_node.put<std::string>(key_str, "TRUE");
                break;
            case instrumentation::BOTH:
                coverage_node.put<std::string>(key_str, "BOTH");
                break;
            default:;
        }
    }

    root.add_child("coverage_map", coverage_node);

    write_json(filename, root, std::locale(), true);
}

std::vector<size_t> calculate_waves(size_t limit, size_t test_count) {
    if (test_count == 0) return {};

    if (limit / test_count < TEST_MIN_DURATION) {
        return {TEST_MIN_DURATION};
    }

    std::vector<size_t> waves;
    const size_t r = TEST_DURATION_INCREASE_RATIO;
    size_t remaining = limit;
    size_t current = TEST_MIN_DURATION;

    while (waves.size() < TEST_MAX_WAVE_COUNT) {
        const size_t wave_time = current;
        const size_t total_consumed = wave_time * test_count;

        if (total_consumed > remaining) {
            if (!waves.empty()) {
                waves.back() += remaining / test_count;
            }
            break;
        }

        waves.push_back(wave_time);
        remaining -= total_consumed;
        current *= r;
    }

    if (waves.empty()) waves.push_back(TEST_MIN_DURATION);

    return waves;
}

void run_test_suite()
{
    const auto executor = std::make_shared<connection::target_executor>(
        get_program_options()->value("path_to_target"));

    auto tests = TestDirParser::parse_dir(get_program_options()->value("test_dir"));


    TestType test_type = get_program_options()->value("goal") == "coverage" ? BRANCH_COVERAGE : ERROR_CALL;
    run_analyzer analyzer(test_type);

    std::cout << "Test count: " << tests.size() << "\nTest type: " << get_program_options()->value("goal") << std::endl;

    uint64_t max_exec_megabytes = boost::lexical_cast<size_t>(get_program_options()->value("max_exec_megabytes"));
    executor->init_shared_memory(1024 * 1024 * max_exec_megabytes);

    auto time_limit = boost::lexical_cast<size_t>(get_program_options()->value("max_exec_milliseconds"));

    for (const auto limit : calculate_waves(time_limit, tests.size())) {

        executor->set_timeout(limit);
        std::cout << "Running test with timeout: " << limit << std::endl;

        for (auto test_buf_it = tests.begin(); test_buf_it != tests.end();) {
            auto size = test_buf_it->size();

            executor->get_shared_memory().clear();
            executor->get_shared_memory() << (uint16_t) max_exec_megabytes;
            executor->get_shared_memory() << (uint64_t) size;
            executor->get_shared_memory().accept_bytes(test_buf_it->data(),
                                                       size);

            executor->execute_target();

            analyzer.add_execution(executor->get_shared_memory());

            if (executor->get_shared_memory().get_termination() ==
                       instrumentation::target_termination::normal) {
                tests.erase(test_buf_it++);
            } else {
                ++test_buf_it;
            }
        }
    }

    auto results = analyzer.get_result();

    std::cout << "Coverage: " << std::setprecision(4) << results.first << std::endl;

    auto result_file =
        std::filesystem::path(get_program_options()->value("output_dir"))
            .append("result.json");

    if (test_type == ERROR_CALL) {
        save_results_to_json(result_file, {results.first, results.second});
    }
    else {
        save_results_to_json(result_file, results);
    }
}

void run(int argc, char* argv[])
{
    if (get_program_options()->value("output_dir").empty()) {
        std::cerr << "ERROR: The output directory path is empty.\n";
        return;
    }

    std::filesystem::path output_dir =
        std::filesystem::absolute(get_program_options()->value("output_dir"));
    {
        std::error_code ec;
        if (ec) {
            std::cerr << "ERROR: Failed to create/access the output "
                         "directory:\n        "
                      << output_dir << "\n";
            return;
        }
    }
    if (get_program_options()->has("clear_output_dir")) {
        for (const auto& entry :
             std::filesystem::directory_iterator(output_dir))
            if (entry.is_regular_file() && entry.path().extension() == ".json")
                std::filesystem::remove(entry);
        if (std::filesystem::is_directory(output_dir / "test-suite"))
            for (const auto& entry :
                 std::filesystem::directory_iterator(output_dir / "test-suite"))
                std::filesystem::remove(entry);
        if (std::filesystem::is_directory(output_dir / "progress_recording"))
            std::filesystem::remove_all(output_dir / "progress_recording");
    }
    if (!get_program_options()->has("path_to_target")) {
        std::cerr << "ERROR: The path to target is empty.\n";
        return;
    }
    if (!std::filesystem::is_regular_file(
            get_program_options()->value("path_to_target"))) {
        std::cerr << "ERROR: The passed target path '"
                  << get_program_options()->value("path_to_target")
                  << "' does not reference a regular file.\n";
        return;
    }
    std::filesystem::perms const perms =
        std::filesystem::status(get_program_options()->value("path_to_target"))
            .permissions();
    if ((perms & std::filesystem::perms::owner_exec) ==
        std::filesystem::perms::none) {
        std::cerr << "ERROR: The passed target path '"
                  << get_program_options()->value("path_to_target")
                  << "' references a file which is NOT executable.\n";
        return;
    }
    if (get_program_options()->has("path_to_client")) {
        if (!std::filesystem::is_regular_file(
                get_program_options()->value("path_to_client"))) {
            std::cerr << "ERROR: The passed client path '"
                      << get_program_options()->value("path_to_client")
                      << "' does not reference a regular file.\n";
            return;
        }
        std::filesystem::perms const perms =
            std::filesystem::status(
                get_program_options()->value("path_to_client"))
                .permissions();
        if ((perms & std::filesystem::perms::owner_exec) ==
            std::filesystem::perms::none) {
            std::cerr << "ERROR: The passed client path '"
                      << get_program_options()->value("path_to_client")
                      << "' references a file which is NOT executable.\n";
            return;
        }
    }

    iomodels::iomanager::instance().set_config(
        {.max_exec_milliseconds = (natural_16_bit)std::max(
             0,
             std::stoi(get_program_options()->value("max_exec_milliseconds"))),
         .max_exec_megabytes = (natural_16_bit)std::max(
             0,
             std::stoi(get_program_options()->value("max_exec_megabytes")))});

    std::string target_name =
        std::filesystem::path(get_program_options()->value("path_to_target"))
            .filename()
            .string();
    {
        std::string const target_suffix       = "_target";
        std::string::size_type const suffix_i = target_name.find(target_suffix);
        if (suffix_i != std::string::npos) {
            target_name.erase(suffix_i, target_suffix.length());
        }
    }

    run_test_suite();
}
