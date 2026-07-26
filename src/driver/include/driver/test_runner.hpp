#pragma once

#include <driver/program_options.hpp>
#include <driver/run_analyzer.hpp>
#include <driver/result_writer.hpp>
#include <driver/test_parser.hpp>
#include <connection/target_executor.hpp>
#include <filesystem>
#include <memory>
#include <vector>

class test_runner
{
public:
    explicit test_runner(program_options_ptr options);

    void run();

private:
    void validate_options() const;
    void prepare_output_directory();
    void configure_io_limits() const;

    TestType                determine_test_type() const;
    TestDirParser::tests    load_test_suite() const;
    void                    execute_waves(connection::target_executor& executor,
                                          TestDirParser::tests& tests,
                                          IRunAnalyzer& analyzer,
                                          const std::vector<size_t>& waves);
    void                    write_test_input(connection::shared_memory& shm,
                                             const TestBuffer& test);
    test_result             collect_results(IRunAnalyzer& analyzer) const;
    void                    report_results(const test_result& result, TestType test_type) const;

    program_options_ptr   options_;
    std::filesystem::path output_dir_;
    size_t                max_exec_megabytes_;
};
