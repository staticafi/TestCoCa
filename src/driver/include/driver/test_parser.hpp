#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>

#include "instrumentation/instrumentation_types.hpp"
#include "utility/math.hpp"

class Parser {
public:
    using Test = std::vector<vecu8>;

    explicit Parser(const std::string& test_dir);

    const std::vector<vecu8>& get_inputs() const;

    class FileReadError : public std::runtime_error { /*...*/ };
    class ParseError : public std::runtime_error { /*...*/ };

private:
    std::string test_dir;
    std::vector<vecu8> inputs;

    void parse();
    void parse_test(const std::filesystem::path& test);
};
