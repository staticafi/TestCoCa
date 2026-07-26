#pragma once

#include <driver/result_writer.hpp>
#include <connection/shared_memory.hpp>
#include <memory>

enum TestType {
    BRANCH_COVERAGE,
    TESTCOMP_COVERAGE,
    ERROR_CALL
};

class IRunAnalyzer {
public:
    virtual ~IRunAnalyzer() = default;
    virtual void add_execution(connection::shared_memory& src) = 0;
    virtual double get_result() const = 0;
    virtual coverage_map get_coverage_map() const = 0;
    virtual void reset() = 0;
};

std::unique_ptr<IRunAnalyzer> create_run_analyzer(TestType test_type);
