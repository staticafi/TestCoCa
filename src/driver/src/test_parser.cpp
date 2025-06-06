#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <driver/test_parser.hpp>
#include <filesystem>
#include <iostream>
#include <boost/filesystem.hpp>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using namespace instrumentation;

TestBuffer::TestBuffer() : buffer(1000)
{
}

const uint8_t *TestBuffer::data() const
{
    return buffer.data();
}

uint64_t TestBuffer::byte_count() const
{
    return offset;
}

uint64_t TestBuffer::input_count() const {
    return inputs;
}



void TestBuffer::write(auto val, type_of_input_bits type)
{
    if (buffer.size() < offset + sizeof(val) + 1) {
        buffer.resize(buffer.size() * 2);
    }

    ++inputs;

    buffer[offset++] = (char) type;
    memcpy(buffer.data() + offset, &val, sizeof(val));
    offset += sizeof(val);
}

namespace TestDirParser {

tests parse_dir(const std::string &test_dir)
{
    bool has_metadata = false;
    tests tests;

    for (const auto& file : std::filesystem::recursive_directory_iterator(test_dir)) {
        if (file.is_regular_file() && file.path().extension() == ".xml") {
            if (!has_metadata && file.path().filename().string() == "metadata.xml") {
                has_metadata = true;
                continue;
            }
            tests.emplace(parse_test(file.path()));
        }
    }

    if (!has_metadata) throw std::runtime_error("Test-suite does not contain metadata.xml file");

    return tests;
}

unsigned __int128 parse_uint128(const std::string &value_str) {
    unsigned __int128 val = 0;
    for (int i = 0; i < value_str.size(); i++) {
        char c = value_str[i];

        if (i == 0 && c == '-') continue;

        if (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
        } else {
            throw std::invalid_argument("Invalid character '" + value_str + "'");
        }
    }
    return val;
}

std::string hex_to_dec_str(const std::string& hex_str) {
    boost::multiprecision::cpp_int num;
    try {
        num = boost::multiprecision::cpp_int(hex_str);
    } catch (...) {
        return "Invalid hex string";
    }

    return num.str();
}

void write_untyped(TestBuffer& buffer, const std::string& value_str) {
    if (value_str[0] == '-') {
        try {
            int16_t val = boost::lexical_cast<int16_t>(value_str);
            buffer.write(val, type_of_input_bits::SINT16);
            return;
        } catch (boost::bad_lexical_cast&) {}
        try {
            int64_t val = boost::lexical_cast<int64_t>(value_str);
            buffer.write(val, type_of_input_bits::SINT64);
            return;
        } catch (boost::bad_lexical_cast&) {}
        try {
            __int128 val = parse_uint128(value_str);
            val *= value_str[0] == '-' ? -1 : 1; //UB??
            buffer.write(val, type_of_input_bits::SINT128);
            return;
        } catch (...) {}
    }
    else {
        try {
            uint16_t val = boost::lexical_cast<uint16_t>(value_str);
            buffer.write(val, type_of_input_bits::UINT16);
            return;
        } catch (boost::bad_lexical_cast&) {}
        try {
            uint64_t val = boost::lexical_cast<uint64_t>(value_str);
            buffer.write(val, type_of_input_bits::UINT64);
            return;
        } catch (boost::bad_lexical_cast&) {}
        try {
            unsigned __int128 val = parse_uint128(value_str);
            buffer.write(val, type_of_input_bits::UINT128);
            return;
        } catch (...) {}
    }
    try {
        double val = boost::lexical_cast<double>(value_str);
        buffer.write(val, type_of_input_bits::FLOAT64);
        return;
    } catch (boost::bad_lexical_cast&) {}

    std::cout << "Failed to parse: " + value_str << ". Value will be saved as 0." << std::endl;
    buffer.write(0, type_of_input_bits::UINT8);
}

void write_typed(TestBuffer& buffer, const std::string& type_str,
                           const std::string& value_str)
{
    try {
        if (type_str == "bool" || type_str == "_Bool") {
            buffer.write(boost::lexical_cast<bool>(value_str), type_of_input_bits::UINT8); return;
        }
        if (type_str == "char") {
            buffer.write(static_cast<char>(boost::lexical_cast<int>(value_str)), type_of_input_bits::SINT8); return;
        }
        if (type_str == "uchar" || type_str == "unsigned char") {
            buffer.write(static_cast<unsigned char>(boost::lexical_cast<unsigned int>(value_str)), type_of_input_bits::UINT8); return;
        }
        if (type_str == "short") {
            buffer.write(boost::lexical_cast<short>(value_str), type_of_input_bits::SINT16); return;
        }
        if (type_str == "ushort" || type_str == "unsigned short") {
            buffer.write(boost::lexical_cast<unsigned short>(value_str), type_of_input_bits::UINT16); return;
        }
        if (type_str == "int") {
            buffer.write(boost::lexical_cast<int>(value_str), type_of_input_bits::SINT32); return;
        }
        if (type_str == "uint" || type_str == "unsigned int" || type_str == "u32") {
            buffer.write(boost::lexical_cast<unsigned int>(value_str), type_of_input_bits::UINT32); return;
        }
        if (type_str == "long") {
            buffer.write(boost::lexical_cast<long>(value_str), sizeof(long) == 4 ? type_of_input_bits::SINT32 : type_of_input_bits::SINT64); return;
        }
        if (type_str == "ulong" || type_str == "unsigned long") {
            buffer.write(boost::lexical_cast<unsigned long>(value_str), sizeof(long) == 4 ? type_of_input_bits::UINT32 : type_of_input_bits::UINT64); return;
        }
        if (type_str == "longlong") {
            buffer.write(boost::lexical_cast<long long>(value_str), sizeof(long long) == 4 ? type_of_input_bits::SINT32 : type_of_input_bits::SINT64); return;
        }
        if (type_str == "ulonglong" || type_str == "unsigned long long") {
            buffer.write(boost::lexical_cast<unsigned long long>(value_str), sizeof(long long) == 4 ? type_of_input_bits::UINT32 : type_of_input_bits::UINT64); return;
        }
        if (type_str == "float") {
            buffer.write(boost::lexical_cast<float>(value_str), type_of_input_bits::FLOAT32); return;
        }
        if (type_str == "double") {
            buffer.write(boost::lexical_cast<double>(value_str), type_of_input_bits::FLOAT64); return;
        }
        if (type_str == "size_t") {
            buffer.write(boost::lexical_cast<size_t>(value_str), sizeof(size_t) == 4 ? type_of_input_bits::UINT32 : type_of_input_bits::UINT64); return;
        }
        if (type_str == "loff_t") {
            buffer.write(boost::lexical_cast<loff_t>(value_str), sizeof(loff_t) == 4 ? type_of_input_bits::SINT32 : type_of_input_bits::SINT64); return;
        }
        if (type_str == "sector_t") {
            buffer.write(boost::lexical_cast<uint64_t>(value_str), type_of_input_bits::UINT64); return;
        }
        if (type_str == "pthread_t") {
            buffer.write(boost::lexical_cast<pthread_t>(value_str), sizeof(loff_t) == 4 ? type_of_input_bits::UINT32 : type_of_input_bits::UINT64); return;
        }
        if (type_str == "pchar") {
            buffer.write(boost::lexical_cast<uint64_t>(value_str), type_of_input_bits::UINT64); return;
        }
        if (type_str == "int128" || type_str == "__int128") {
            __int128 num = parse_uint128(value_str);
            num *= value_str[0] == '-' ? -1 : 1; //UB??
            buffer.write(num, type_of_input_bits::SINT128);
            return;
        }
        if (type_str == "uint128" || type_str == "unsigned int128" || type_str == "unsigned __int128") {
            unsigned __int128 num = parse_uint128(value_str);
            buffer.write(num, type_of_input_bits::UINT128);
            return;
        }

    } catch (const boost::bad_lexical_cast&) {}

    std::cout << "Failed to parse " + value_str + " as " + type_str << ". Attempting to parse with deduced type." << std::endl;
    write_untyped(buffer, value_str);
}

TestBuffer parse_test(const std::filesystem::path& test_path)
{
    boost::property_tree::ptree pt;

    try {
        read_xml(test_path, pt);
    } catch (const boost::property_tree::xml_parser_error& e) {
        std::cout << e.what() << std::endl;
    }

    TestBuffer buffer;
    uint64_t input_count = 0;

    const auto& testcase = pt.get_child_optional("testcase");

    if (!testcase) {
        return buffer;
    }

    for (const auto& [name, node] : *testcase) {
        if (name != "input") continue;

        auto type_str = node.get<std::string>("<xmlattr>.type", "");

        auto data = node.data();

        boost::trim(data);

        if (data.length() == 3 && ( data[0] == '\'' && data[2]== '\'') || ( data[0] == '\"' && data[2]== '\"')) {
            data = std::to_string(data[1]);
        }

        if (data.starts_with("0x")) {
            data = hex_to_dec_str(data);
        }

        if (type_str.empty()) {
            write_untyped(buffer, data);
        } else {
            write_typed(buffer, type_str, data);
        }

        ++input_count;
    }

    return buffer;
}
}  // namespace TestParser
