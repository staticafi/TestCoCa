#include "driver/run_analyzer.hpp"

#include <numeric>
#include <boost/property_tree/json_parser.hpp>

typedef uint64_t checksum_t;
/**
 **/

class BranchCoverageAnalyzer : public IRunAnalyzer {
    uint32_t br_instr_count = 0;
    coverage_map coverage;

public:
    void add_execution(connection::shared_memory &src) override {
        if (br_instr_count == 0) {
            br_instr_count = src.get_cond_br_count();
        } else if (br_instr_count != src.get_cond_br_count()) {
            std::cout << "WARNING: number of total reported br instructions differs between test runs" << std::endl;
        }

        unsigned char *mem = src.get_memory() +
                             sizeof(instrumentation::target_termination) +
                             sizeof(checksum_t) +
                             sizeof(uint32_t) +
                             sizeof(uint32_t);

        checksum_t hash = 0;

        for (auto *br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem);
             br_info->id != 0;
             mem += sizeof(instrumentation::br_instr_coverage_info),
             br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem)) {
            hash += br_info->id;
            hash += br_info->coverage;

            if (br_info->id > br_instr_count || br_info->coverage > instrumentation::BOTH) {
                std::cout <<
                        "WARNING: br instruction id or coverage is outside expected limits and will not be added to the final result."
                        << std::endl;
                continue;
            }

            if (auto it = coverage.find(br_info->id); it != coverage.end()) {
                if (it->second != instrumentation::BOTH &&
                    it->second != br_info->coverage) {
                    it->second = instrumentation::BOTH;
                }
            } else {
                coverage.emplace(br_info->id, br_info->coverage);
            }
        }

        if (hash != *src.checksum()) {
            std::cout << "WARNING: run result data checksum failed" << std::endl;
        }
    }

    void reset() override {
        coverage.clear();
        br_instr_count = 0;
    }

    double get_result() override {
        uint64_t conditions_covered = 0;
        for (auto &[id, cc]: coverage) {
            conditions_covered += cc == instrumentation::BOTH ? 2 : 1;
        }

        assert(conditions_covered <= br_instr_count * 2);

        std::cout << "Total: " << br_instr_count * 2 << std::endl;
        std::cout << "Covered: " << conditions_covered << std::endl;


        if (br_instr_count == 0 || conditions_covered == 0) {
            return 0.0;
        }

        return conditions_covered / static_cast<long double>(br_instr_count * 2);
    }

    TestType get_type() const override {
        return BRANCH_COVERAGE;
    }
};



class TestCompCoverageAnalyzer : public IRunAnalyzer {
    uint32_t goal_count = 0;
    std::vector<uint8_t> coverage;

public:
    void add_execution(connection::shared_memory &src) override {
        if (goal_count == 0) {
            goal_count = src.get_goal_count();
            coverage.resize(goal_count);
        } else if (goal_count != src.get_goal_count()) {
            std::cout << "WARNING: number of total reported goals differs between test runs" << std::endl;
        }

        unsigned char *mem = src.get_memory() +
                             sizeof(instrumentation::target_termination) +
                             sizeof(checksum_t) +
                             sizeof(uint32_t) +
                             sizeof(uint32_t);

        for (int i = 0; i < goal_count; i++) {
            coverage[i] |= mem[i];
        }
    }

    void reset() override {
        coverage.clear();
        goal_count = 0;
    }

    double get_result() override {
        uint32_t goals_hit = std::accumulate(coverage.begin(), coverage.end(), 0);

        std::cout << "Total: " << goal_count << std::endl;
        std::cout << "Covered: " << goals_hit << std::endl;

        if (goal_count == 0) return goals_hit == 0 ? 1.0 : 0.0;

        return (double) goals_hit / goal_count;
    }

    TestType get_type() const override {
        return BRANCH_COVERAGE;
    }
};



class ErrorCallAnalyzer : public IRunAnalyzer {
    bool goal_reached = false;

public:
    void add_execution(connection::shared_memory &src) override {
        if (src.get_termination() ==
            instrumentation::target_termination::ver_error_reached) {
            goal_reached = true;
        }
    }

    void reset() override {
        goal_reached = false;
    }

    double get_result() override {
        return goal_reached;
    }

    TestType get_type() const override {
        return ERROR_CALL;
    }
};

std::unique_ptr<IRunAnalyzer> create_run_analyzer(TestType test_type) {
    switch (test_type) {
        case BRANCH_COVERAGE:
            return std::make_unique<BranchCoverageAnalyzer>();
        case TESTCOMP_COVERAGE:
            return std::make_unique<TestCompCoverageAnalyzer>();
        case ERROR_CALL:
            return std::make_unique<ErrorCallAnalyzer>();
        default:
            throw std::invalid_argument("Unknown test type");
    }
}
