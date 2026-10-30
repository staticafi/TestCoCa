#pragma once

#include <optional>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/functional/hash.hpp>
#include <boost/unordered_set.hpp>

#include "target/instrumentation_types.hpp"

class TestBuffer {
    std::vector<uint8_t> buffer;
    uint64_t offset = 0;
    uint64_t inputs = 0;

public:
    TestBuffer();
    void write(auto val, instrumentation::type_of_input_bits type);
    uint64_t byte_count() const;
    uint64_t input_count() const;
    const uint8_t *data() const;

    bool operator==(const TestBuffer& other) const {
        return buffer == other.buffer && offset == other.offset && inputs == other.inputs;
    }

    friend size_t hash_value(const TestBuffer& buf) {
        return boost::hash_range(buf.buffer.begin(), buf.buffer.end());
    }
};

namespace TestDirParser {
struct test_case {
    TestBuffer input;
    std::vector<std::string> command_line_args;
    std::optional<std::string> stdin_data;

    bool operator==(const test_case& other) const {
        return input == other.input
            && command_line_args == other.command_line_args
            && stdin_data == other.stdin_data;
    }

    friend size_t hash_value(const test_case& test) {
        size_t seed = hash_value(test.input);
        boost::hash_combine(
            seed,
            boost::hash_range(test.command_line_args.begin(),
                              test.command_line_args.end()));
        boost::hash_combine(seed, test.stdin_data.has_value());
        if (test.stdin_data) {
            boost::hash_combine(
                seed,
                boost::hash_range(test.stdin_data->begin(),
                                  test.stdin_data->end()));
        }
        return seed;
    }
};

using tests = boost::unordered_set<test_case>;

tests parse_dir(const std::string& test_dir);

test_case parse_test(const std::filesystem::path& test_path);
}
