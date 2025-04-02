#pragma once

#include <utility/math.hpp>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

namespace iomodels {

struct input_model {
    using byte_count_type    = uint64_t;
    using type_of_input_bits = instrumentation::type_of_input_bits;
    using input_types_vector = std::vector<type_of_input_bits>;

    input_model(byte_count_type max_bytes_);

    void clear();
    void save(connection::shared_memory& dest) const;
    void load(connection::shared_memory& src);
    byte_count_type max_bytes() const;

    vecu8 const& get_bytes() const
    {
        return bytes;
    }

    input_types_vector const& get_types() const
    {
        return types;
    }

    byte_count_type num_bytes_read() const
    {
        return cursor;
    }

    void set_bytes(vecu8 const& bytes_)
    {
        bytes = bytes_;
    }

    template <typename T>
    void read(T* ptr, connection::shared_memory& dest)
    {
        auto const type = (type_of_input_bits) bytes[cursor++];
        uint8_t const count = num_bytes(type);

        if (cursor + count > max_bytes()) {
            dest.set_termination(
                instrumentation::target_termination::boundary_condition_violation);
            exit(0);
        }
        if (!dest.can_accept_bytes(count + 2)) {
            dest.set_termination(instrumentation::target_termination::medium_overflow);
            exit(0);
        }
        if (cursor + count > bytes.size()) {
            dest.set_termination(instrumentation::target_termination::insufficient_data);
            exit(0);
        }

        switch (type) {
            case type_of_input_bits::UINT8:
                *ptr = *(uint8_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT8:
                *ptr = *(int8_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT16:
                *ptr = *(uint16_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT16:
                *ptr = *(int16_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT32:
                *ptr = *(uint32_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT32:
                *ptr = *(int32_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT64:
                *ptr = *(uint64_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT64:
                *ptr = *(int64_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::FLOAT32:
                *ptr = *(float*) (bytes.data() + cursor); break;
            case type_of_input_bits::FLOAT64:
                *ptr = *(double*) (bytes.data() + cursor); break;
            default: ;
        }

        cursor += count;
    }


   private:
    byte_count_type m_max_bytes;
    byte_count_type cursor;
    vecu8 bytes;
    input_types_vector types;
};

using input_model_ptr = std::unique_ptr<input_model>;

}  // namespace iomodels
