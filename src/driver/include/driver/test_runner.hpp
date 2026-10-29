#pragma once

#include <driver/program_options.hpp>
#include <driver/run_analyzer.hpp>
#include <driver/result_writer.hpp>
#include <driver/test_parser.hpp>
#include <driver/time_budget.hpp>
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
    enum class execution_stop_reason
    {
        completed,
        time_budget_depleted,
        budget_too_small
    };

    struct execution_summary
    {
        std::size_t waves = 0;
        std::size_t executed = 0;
        std::size_t retried = 0;
        std::size_t skipped = 0;
        bool time_limit_reached = false;
        execution_stop_reason reason = execution_stop_reason::completed;
    };

    void validate_options() const;
    void prepare_output_directory();
    void configure_io_limits() const;

    TestType                determine_test_type() const;
    TestDirParser::tests    load_test_suite() const;
    execution_summary       execute_waves(connection::target_executor& executor,
                                          TestDirParser::tests& tests,
                                          IRunAnalyzer& analyzer,
                                          const time_budget& budget);
    void                    write_test_input(connection::shared_memory& shm,
                                             const TestDirParser::test_case& test);
    test_result             collect_results(IRunAnalyzer& analyzer) const;
    void                    report_results(const test_result& result, TestType test_type) const;
    void                    report_time_budget(const time_budget& budget,
                                               const execution_summary& summary) const;

    program_options_ptr   options_;
    std::filesystem::path output_dir_;
    size_t                max_exec_megabytes_;
};
