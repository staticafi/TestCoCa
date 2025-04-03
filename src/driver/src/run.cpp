#include <boost/process/exe.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <connection/target_executor.hpp>
#include <driver/program_options.hpp>
#include <filesystem>
#include <iomodels/iomanager.hpp>
#include <iostream>

#include "driver/run_analyzer.hpp"
#include "driver/test_parser.hpp"

void save_results_to_json(const std::string& filename, const std::pair<double, coverage_map>& results)
{
    boost::property_tree::ptree root;

    root.put<float>("result", results.first);

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

    root.add_child("coverage", coverage_node);

    write_json(filename, root, std::locale(), true);
}

void run_test_suite()
{
    const auto executor = std::make_shared<connection::target_executor>(
        get_program_options()->value("path_to_target"));

    auto [test_type, tests] =
        TestDirParser::parse_dir(get_program_options()->value("test_dir"));

    std::cout << "tests count: " << tests.size() << std::endl;

    run_analyzer analyzer;

    // TODO size?
    executor->init_shared_memory(1000);

    bool error_reached = false;

    for (const auto limit : {20, 100, 500}) {

        if (error_reached) break;

        executor->set_timeout(limit);

        for (auto test_buf_it = tests.begin(); test_buf_it != tests.end();) {
            std::cout << "Running tests with timeout " << limit << std::endl;
            auto size = test_buf_it->size();

            executor->get_shared_memory().clear();

            executor->get_shared_memory() << (uint64_t)size;
            executor->get_shared_memory().accept_bytes(test_buf_it->data(),
                                                       size);

            executor->execute_target();

            analyzer.add_execution(executor->get_shared_memory());

            if (test_type == TestDirParser::CALL &&
                executor->get_shared_memory().get_termination() ==
                    instrumentation::target_termination::ver_error_reached) {
                error_reached = true;
                break;
            }

            if (executor->get_shared_memory().get_termination() ==
                       instrumentation::target_termination::normal) {
                tests.erase(test_buf_it++);
            } else {
                ++test_buf_it;
            }
        }
    }

    auto results = analyzer.get_result();

    auto result_file =
        std::filesystem::path(get_program_options()->value("output_dir"))
            .append("result.json");

    if (test_type == TestDirParser::CALL) {
        save_results_to_json(result_file, {error_reached, results.second});
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
