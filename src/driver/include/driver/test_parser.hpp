#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <set>
#include <variant>

#include "target/instrumentation_types.hpp"
#include "utility/math.hpp"

namespace TestDirParser {
    using tests = std::set<vecu8>;
    enum TestType {
        COVERAGE,
        BUG
    };

    std::pair<TestType, tests> parse(const std::string& test_dir);
}



class TestBuffer {
    std::vector<uint8_t> buffer;
    uint64_t offset;

public:
    TestBuffer();
    void write(auto val);
    uint64_t size() const;
    std::vector<uint8_t>& get_buffer() ;
};


/*
class Parser {
public:
    using int128_t = __int128;
    using uint128_t = unsigned __int128;
    using loff_t = int64_t;
    using sector_t = uint64_t;
    using pchar = char*;
    using pthread_t = uint64_t;
    using u32 = uint32_t;

    using InputVariant = std::variant<
#if CPU_TYPE() == CPU64()
        int128_t,
        uint128_t,
#endif
        bool,
        char,
        short,
        unsigned short,
        int,
        unsigned int,
        long,
        unsigned long,
        long long,
        unsigned long long,
        float,
        double
    >;

    using Test = std::vector<vecu8>;

    explicit Parser(const std::string& test_dir);

    InputVariant parse_input_with_type(const std::string& type_str, const std::string& value_str);

    const std::set<vecu8>& get_inputs() const;

private:
    std::string test_dir;
    std::set<vecu8> inputs;

    void parse();
    void append(Test& test, auto val);
    void parse_test(const std::filesystem::path& test_path);
};

*/