#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <driver/test_parser.hpp>
#include <filesystem>
#include <iostream>

TestBuffer::TestBuffer() : offset(0), buffer(100)
{
}

const uint8_t *TestBuffer::data() const
{
    return buffer.data();
}

uint64_t TestBuffer::size() const
{
    return offset;
}

void TestBuffer::write(auto val)
{
    std::cout << "writing value: " << val << std::endl;

    if (buffer.capacity() < offset + sizeof(val) + 1) {
        buffer.reserve(buffer.capacity() * 2);
    }

    buffer[offset++] = sizeof(val);
    memcpy(buffer.data() + offset, &val, sizeof(val));
    offset += sizeof(val);

    for (auto byte: buffer) {
        std::cout << (int) byte << " ";
    }
    std::cout << std::endl;
}

namespace TestDirParser {

std::pair<TestType, tests> parse_dir(
    const std::string& dir_path)
{
    std::cout << "Parsing test directory: " << dir_path << std::endl;

    tests tests;
    for (auto file : std::filesystem::directory_iterator(dir_path)) {
        if (file.path().filename().string().find("test") != std::string::npos) {
            tests.emplace(parse_test(file.path()));
        }
    }

    return {COVERAGE, tests};
}

void write_untyped(TestBuffer& buffer, const std::string& value_str) {
    try {
        uint8_t val = boost::numeric_cast<uint8_t>(boost::lexical_cast<uint16_t>(value_str));
        buffer.write(val);
        return;
    } catch (...) {}
    try {
        uint16_t val = boost::lexical_cast<uint16_t>(value_str);
        buffer.write(val);
        return;
    } catch (boost::bad_lexical_cast&) {}
    try {
        uint32_t val = boost::lexical_cast<uint32_t>(value_str);
        buffer.write(val);
        return;
    } catch (boost::bad_lexical_cast&) {}
    try {
        uint64_t val = boost::lexical_cast<uint64_t>(value_str);
        buffer.write(val);
        return;
    } catch (boost::bad_lexical_cast&) {}
    try {
        unsigned __int128 val = boost::lexical_cast<uint8_t>(value_str);
        buffer.write((uint64_t*)(&val)[0]);
        buffer.write((uint64_t*)(&val)[1]);
        return;
    } catch (boost::bad_lexical_cast&) {}
    try {
        float val = boost::lexical_cast<float>(value_str);
        buffer.write(val);
        return;
    } catch (boost::bad_lexical_cast&) {}
    try {
        double val = boost::lexical_cast<double>(value_str);
        buffer.write(val);
        return;
    } catch (boost::bad_lexical_cast&) {}
    throw std::runtime_error("Failed to parse " + value_str);
}

void write_typed(TestBuffer& buffer, const std::string& type_str,
                           const std::string& value_str)
{
    try {
        if (type_str == "bool")
            buffer.write(boost::lexical_cast<bool>(value_str));
        if (type_str == "char")
            buffer.write((char)boost::lexical_cast<int>(value_str));
        if (type_str == "uchar" || type_str == "unsigned char")
            buffer.write((unsigned char)boost::lexical_cast<unsigned int>(value_str));
        if (type_str == "short")
            buffer.write(boost::lexical_cast<short>(value_str));
        if (type_str == "ushort" || type_str == "unsigned short")
            buffer.write(boost::lexical_cast<unsigned short>(value_str));
        if (type_str == "int")
            buffer.write(boost::lexical_cast<int>(value_str));
        if (type_str == "uint" || type_str == "unsigned int")
            buffer.write(boost::lexical_cast<unsigned int>(value_str));
        if (type_str == "long")
            buffer.write(boost::lexical_cast<long>(value_str));
        if (type_str == "ulong" || type_str == "unsigned long")
            buffer.write(boost::lexical_cast<unsigned long>(value_str));
        if (type_str == "longlong")
            buffer.write(boost::lexical_cast<long long>(value_str));
        if (type_str == "ulonglong" || type_str == "unsigned long long")
            buffer.write(boost::lexical_cast<unsigned long long>(value_str));
        if (type_str == "float")
            buffer.write(boost::lexical_cast<float>(value_str));
        if (type_str == "double")
            buffer.write(boost::lexical_cast<double>(value_str));
        if (type_str == "size_t")
            buffer.write(boost::lexical_cast<size_t>(value_str));
        if (type_str == "loff_t")
            buffer.write(boost::lexical_cast<loff_t>(value_str));
        if (type_str == "sector_t")
            buffer.write(boost::lexical_cast<uint64_t>(value_str));
        if (type_str == "pthread_t")
            buffer.write(boost::lexical_cast<pthread_t>(value_str));
        if (type_str == "u32")
            buffer.write(boost::lexical_cast<uint32_t>(value_str));
        if (type_str == "pchar")
            buffer.write(boost::lexical_cast<uint64_t>(value_str));
        if (type_str == "int128") {
            __int128 num = boost::lexical_cast<__int128>(value_str);
            buffer.write((uint64_t*)(&num)[0]);
            buffer.write((uint64_t*)(&num)[1]);
        }
        if (type_str == "uint128") {
            unsigned __int128 num = boost::lexical_cast<unsigned __int128>(value_str);
            buffer.write((uint64_t*)(&num)[0]);
            buffer.write((uint64_t*)(&num)[1]);
        }
    } catch (const boost::bad_lexical_cast& e) {
        throw std::runtime_error("Failed to parse " + value_str + " as " +
                                 type_str + ": " + e.what());
    }
}

TestBuffer parse_test(const std::filesystem::path& test_path)
{
    boost::property_tree::ptree pt;
    read_xml(test_path, pt);

    TestBuffer buffer;

    const auto& testcase = pt.get_child("testcase");

    for (const auto& [name, node] : testcase) {
        if (name != "input") continue;

        auto type_str = node.get<std::string>("<xmlattr>.type", "");

        if (!type_str.empty()) {
            write_typed(buffer, type_str, node.data());
        } else {
            write_untyped(buffer, node.data());
        }
    }

    return buffer;
}
}  // namespace TestParser
