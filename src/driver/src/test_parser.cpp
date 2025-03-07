#include <driver/test_parser.hpp>
#include <filesystem>
#include <iostream>
#include <boost/property_tree/xml_parser.hpp>

class FileReadError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Parser::Parser(const std::string& test_dir) : test_dir(test_dir) {
    //try {
        parse();
    /*
    } catch (const boost::property_tree::xml_parser_error& e) {
        throw FileReadError("Failed to read XML file: " + std::string(e.what()));
    } catch (const boost::property_tree::ptree_bad_path& e) {
        throw ParseError("Invalid XML structure: " + std::string(e.what()));
    }
    */
}

const std::vector<Parser::Test>& Parser::get_inputs() const {
    return inputs;
}

void Parser::parse() {
    std::cout << "Parsing test directory" << std::endl;
    inputs.clear();

    for (auto file : std::filesystem::directory_iterator(test_dir)) {
        if (file.path().filename().string().find("test") != std::string::npos) {
            parse_test(file);
        }
    }
}

void Parser::parse_test(const std::filesystem::path& test) {
    boost::property_tree::ptree pt;
    read_xml(test, pt);

    const auto& testcase = pt.get_child("testcase");

    std::vector<Input> test_vector;
    for (const auto& [name, node] : testcase) {
        if (name == "input") {
            Input input{
                node.get<std::string>("<xmlattr>.type", ""),
                node.data()
            };
            test_vector.push_back(input);
        }
    }

    inputs.push_back(test_vector);
}