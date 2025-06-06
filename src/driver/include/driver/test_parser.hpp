#pragma once

#include <string>
#include <vector>
#include <filesystem>
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
    using tests = boost::unordered_set<TestBuffer>;

    tests parse_dir(const std::string& test_dir);

    TestBuffer parse_test(const std::filesystem::path& test_path);
}