#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>

class Parser {
public:
    struct Input {
        std::string type;
        std::string value;
    };

    using Test = std::vector<Input>;

    explicit Parser(const std::string& test_dir);

    const std::vector<Test>& get_inputs() const;

    class FileReadError : public std::runtime_error { /*...*/ };
    class ParseError : public std::runtime_error { /*...*/ };

private:
    std::string test_dir;
    std::vector<Test> inputs;

    void parse();
    void parse_test(const std::filesystem::path& test);
};