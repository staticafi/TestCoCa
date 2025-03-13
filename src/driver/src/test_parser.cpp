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

const std::vector<vecu8>& Parser::get_inputs() const {
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

void append(auto& vec, auto type, auto val) {
   // vec.push_back(static_cast<uint8_t>(type));

    for (int i = instrumentation::num_bytes(type); i; --i) {
        auto chunk = static_cast<uint8_t>(val >> (i - 1) * 8);
        vec.push_back(chunk);
    }
}

void Parser::parse_test(const std::filesystem::path& test) {
    boost::property_tree::ptree pt;
    read_xml(test, pt);

    const auto& testcase = pt.get_child("testcase");

    vecu8 test_vector;
    for (const auto& [name, node] : testcase) {
        if (name == "input") {
            instrumentation::type_of_input_bits type = instrumentation::from_string(node.get<std::string>("<xmlattr>.type", "unsigned long"));

            //TODO support for unsigned long long
            auto val = std::stoll(node.data());
            append(test_vector, type,  val);
        }
    }

    std::cout << "parsed data: " << std::endl;
    for (unsigned char i : test_vector) {
        std::cout << (int) i << " ";
    }
    std::cout << std::endl;

    test_vector.shrink_to_fit();

    inputs.push_back(test_vector);
}

