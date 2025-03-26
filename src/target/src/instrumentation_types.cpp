#include <target/instrumentation_types.hpp>
#include <utility/invariants.hpp>
#include <ostream>
#include <iomanip>

namespace  instrumentation {


br_instr_coverage_info::br_instr_coverage_info(const br_instr_id id, const condition_coverage coverage)
    : id(id)
    , coverage(coverage)
{}

size_t br_instr_coverage_info::flattened_size() {
    return sizeof(id) + sizeof(condition_coverage);
}

type_of_input_bits  from_id(natural_8_bit const  id)
{
    switch (id)
    {
        case  0U: return type_of_input_bits::BOOLEAN;

        case  1U: return type_of_input_bits::UINT8;
        case  2U: return type_of_input_bits::SINT8;

        case  3U: return type_of_input_bits::UINT16;
        case  4U: return type_of_input_bits::SINT16;

        case  5U: return type_of_input_bits::UINT32;
        case  6U: return type_of_input_bits::SINT32;

        case  7U: return type_of_input_bits::UINT64;
        case  8U: return type_of_input_bits::SINT64;

        case  9U: return type_of_input_bits::FLOAT32;
        case 10U: return type_of_input_bits::FLOAT64;

        case 11U: return type_of_input_bits::UNTYPED8;
        case 12U: return type_of_input_bits::UNTYPED16;
        case 13U: return type_of_input_bits::UNTYPED32;
        case 14U: return type_of_input_bits::UNTYPED64;

        default: { UNREACHABLE(); return type_of_input_bits::UINT8; }
    }
}

type_of_input_bits from_string(std::string&& type_str) {
    if (type_str == "bool") {return type_of_input_bits::BOOLEAN;}
    if (type_str == "unsigned char") {return type_of_input_bits::UINT8;}
    if (type_str == "char") {return type_of_input_bits::SINT8;}
    if (type_str == "unsigned short") {return type_of_input_bits::UINT16;}
    if (type_str == "short") {return type_of_input_bits::SINT16;}
    if (type_str == "unsigned int") {return type_of_input_bits::UINT32;}
    if (type_str == "int") {return type_of_input_bits::SINT32;}
    if (type_str == "unsigned long") {return type_of_input_bits::UINT64;}
    if (type_str == "long") {return type_of_input_bits::SINT64;}
    if (type_str == "float") {return type_of_input_bits::FLOAT32;}
    if (type_str == "double") {return type_of_input_bits::FLOAT64;}
    UNREACHABLE();
}


natural_8_bit num_bytes(type_of_input_bits type)
{
    switch (type)
    {
        case type_of_input_bits::BOOLEAN:
        case type_of_input_bits::UINT8:
        case type_of_input_bits::SINT8:
        case type_of_input_bits::UNTYPED8:
            return 1U;
        case type_of_input_bits::UINT16:
        case type_of_input_bits::SINT16:
        case type_of_input_bits::UNTYPED16:
            return 2U;
        case type_of_input_bits::UINT32:
        case type_of_input_bits::SINT32:
        case type_of_input_bits::FLOAT32:
        case type_of_input_bits::UNTYPED32:
            return 4U;
        case type_of_input_bits::UINT64:
        case type_of_input_bits::SINT64:
        case type_of_input_bits::FLOAT64:
        case type_of_input_bits::UNTYPED64:
            return 8U;
        default: { UNREACHABLE(); return 0U; }
    }
}

}
