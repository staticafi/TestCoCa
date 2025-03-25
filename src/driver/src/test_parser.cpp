#include <driver/test_parser.hpp>
#include <filesystem>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>

TestBuffer::TestBuffer() : offset(0), buffer(100) {}

std::vector<uint8_t> &TestBuffer::get_buffer(){
    return buffer;
}

uint64_t TestBuffer::size() const {
    return offset;
}

void TestBuffer::write(auto val) {
    if (buffer.capacity() < offset + sizeof(val)) {
        buffer.reserve(buffer.capacity() * 2);
    }

    buffer.data()[offset] = val;
    offset += sizeof(val);
}

namespace TestParser {
    std::pair<TestDirParser::TestType, TestDirParser::tests> parse(const std::string &test_dir) {

        std::cout << "Parsing test directory: " << test_dir << std::endl;

        for (auto file : std::filesystem::directory_iterator(test_dir)) {
            if (file.path().filename().string().find("test") != std::string::npos) {
                parse_test(file);
            }
        }

        return result;
    }
}












/*
Parser::Parser(const std::string& test_dir) : test_dir(test_dir) {
    std::cout << "Parsing test directory: " << test_dir << std::endl;
    inputs.clear();

    for (auto file : std::filesystem::directory_iterator(test_dir)) {
        if (file.path().filename().string().find("test") != std::string::npos) {
            parse_test(file);
        }
    }
}

const std::set<vecu8>& Parser::get_inputs() const {
    return inputs;
}

void Parser::parse() {
}

void Parser::append(Test& test, auto val) {
    if (test.capacity() > test.size() + sizeof(val)) test.reserve(test.capacity() * 2);

    for (int i = sizeof(val); i; --i) {
        auto chunk = static_cast<uint8_t*>(val)[i - 1];
        cont.push_back(chunk);
    }
}

Parser::InputVariant Parser::parse_input_with_type(const std::string& type_str, const std::string& value_str) {
    try {
        if (type_str == "bool")   return boost::lexical_cast<bool>(value_str);
        if (type_str == "char")   return boost::lexical_cast<char>(value_str);
        if (type_str == "short")  return boost::lexical_cast<short>(value_str);
        if (type_str == "ushort") return boost::lexical_cast<unsigned short>(value_str);
        if (type_str == "int")    return boost::lexical_cast<int>(value_str);
        if (type_str == "uint")   return boost::lexical_cast<unsigned int>(value_str);
        if (type_str == "long")   return boost::lexical_cast<long>(value_str);
        if (type_str == "ulong")  return boost::lexical_cast<unsigned long>(value_str);
        if (type_str == "longlong") return boost::lexical_cast<long long>(value_str);
        if (type_str == "ulonglong") return boost::lexical_cast<unsigned long long>(value_str);
        if (type_str == "float")  return boost::lexical_cast<float>(value_str);
        if (type_str == "double") return boost::lexical_cast<double>(value_str);
        if (type_str == "size_t") return boost::lexical_cast<size_t>(value_str);


        if (type_str == "loff_t")   return boost::lexical_cast<loff_t>(value_str);
        if (type_str == "sector_t") return boost::lexical_cast<sector_t>(value_str);
        if (type_str == "pthread_t") return boost::lexical_cast<pthread_t>(value_str);
        if (type_str == "u32")       return boost::lexical_cast<u32>(value_str);

        //TODO??
#if CPU_TYPE() == CPU64()
        if (type_str == "int128")  return boost::lexical_cast<int128_t>(value_str);
        if (type_str == "uint128") return boost::lexical_cast<uint128_t>(value_str);

        if (type_str == "pchar") return boost::lexical_cast<uint64_t>(value_str);
#else
        if (type_str == "pchar") return boost::lexical_cast<uint32_t>(value_str);
#endif

    }
    catch (const boost::bad_lexical_cast& e) {
        throw std::runtime_error("Failed to parse " + value_str + " as " + type_str + ": " + e.what());
    }

    throw std::runtime_error("Unknown type: " + type_str);
}


void Parser::parse_test(const std::filesystem::path& test) {
    boost::property_tree::ptree pt;
    read_xml(test, pt);

    const auto& testcase = pt.get_child("testcase");

    vecu8 test_vector;
    for (const auto& [name, node] : testcase) {
        if (name != "input") continue;

        auto type_str = node.get<std::string>("<xmlattr>.type", "");

        if (!type_str.empty()) {
            auto val = parse_input_with_type(node.data(), type_str);
            append(test_vector, val);
            continue;
        }

        #instrumentation::type_of_input_bits type = instrumentation::from_string(node.get<std::string>("<xmlattr>.type", "unsigned long"));

        #auto val = std::stoll(node.data());
        #append(test_vector, val);


    }

    std::cout << "parsed data: " << std::endl;
    for (unsigned char i : test_vector) {
        std::cout << (int) i << " ";
    }
    std::cout << std::endl;

    test_vector.shrink_to_fit();

    inputs.emplace(test_vector);
}

        */
