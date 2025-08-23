#pragma once

#include <unordered_map>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

enum TestType {
    BRANCH_COVERAGE,
    TESTCOMP_COVERAGE,
    ERROR_CALL
};

using coverage_map = std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage>;

class IRunAnalyzer {
public:
    virtual ~IRunAnalyzer() = default;
    virtual void add_execution(connection::shared_memory& src) = 0;
    virtual double get_result() = 0;
    virtual void reset() = 0;
    virtual TestType get_type() const = 0;
};

std::unique_ptr<IRunAnalyzer> create_run_analyzer(TestType test_type);
