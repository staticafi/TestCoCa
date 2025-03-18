#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <set>

#include "target/instrumentation_types.hpp"
#include "utility/math.hpp"

class Parser {
public:
    using Test = std::vector<vecu8>;

    explicit Parser(const std::string& test_dir);

    const std::set<vecu8>& get_inputs() const;

    //TODO
    class FileReadError : public std::runtime_error { /*...*/ };
    class ParseError : public std::runtime_error { /*...*/ };

private:
    std::string test_dir;
    std::set<vecu8> inputs;

    void parse();
    void parse_test(const std::filesystem::path& test);
};
