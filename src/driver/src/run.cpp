#include <connection/target_executor.hpp>
#include <iomodels/iomanager.hpp>
#include <iostream>
#include <driver/program_options.hpp>
#include <filesystem>
#include <boost/process/exe.hpp>

#include "driver/test_parser.hpp"

void run(int argc, char* argv[]) {
    const std::string& test_type = get_program_options()->value("test_type");
    if (test_type != "native" && test_type != "testcomp") {
        std::cerr << "ERROR: unknown output type specified. Use native or "
                     "testcomp.\n";
        return;
    }

    if (get_program_options()->value("output_dir").empty()) {
        std::cerr << "ERROR: The output directory path is empty.\n";
        return;
    }
    std::filesystem::path output_dir =
        std::filesystem::absolute(get_program_options()->value("output_dir"));
    {
        std::error_code ec;
        if (test_type == "testcomp") {
            std::filesystem::create_directories(output_dir / "test-suite", ec);
        } else {
            std::filesystem::create_directories(output_dir, ec);
        }
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
             0, std::stoi(get_program_options()->value("max_exec_megabytes"))),
         .stdin_model_name  = get_program_options()->value("stdin_model")});

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

    std::shared_ptr<connection::target_executor> executor;
    std::cout << "Communication type: shared memory" << std::endl;

    executor = std::make_shared<connection::target_executor>(
        get_program_options()->value("path_to_target"));

    Parser parser(get_program_options()->value("test_dir"));

    executor->init_shared_memory(100);

    std::set inputs = parser.get_inputs();
    for (auto to: {20, 100, 200}) {
        executor->set_timeout(to);

        std::cout << "Running tests with timeout " << to << std::endl;
        for (auto test_vec_it = inputs.begin(); test_vec_it != inputs.end();) {
            auto size = test_vec_it->size();

            executor->get_shared_memory().clear();

            executor->get_shared_memory() << static_cast<natural_32_bit>(size);
            executor->get_shared_memory().accept_bytes(test_vec_it->data(), size);
            std::cout << "test loaded into shared memory" << std::endl;

            executor->get_shared_memory().print();
            executor->execute_target();
            executor->get_shared_memory().print();

            if (executor->get_shared_memory().get_termination() == instrumentation::target_termination::normal ||
                executor->get_shared_memory().get_termination() == instrumentation::target_termination::ver_error_reached)
            {
                std::cout << "tu som" << std::endl;
                inputs.erase(test_vec_it++);
            } else {
                ++test_vec_it;
            }

        }
    }
}
