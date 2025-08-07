#pragma once

#include <utility/math.hpp>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

namespace iomodels {

struct input_model {
    using type_of_input_bits = instrumentation::type_of_input_bits;

    input_model(uint64_t max_bytes_);

    void clear();
    void load(connection::shared_memory& src);
    uint64_t max_bytes() const;

    vecu8 const& get_bytes() const
    {
        return bytes;
    }

    uint64_t num_bytes_read() const
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
        if (read_inputs >= input_count) {
            dest.set_termination(instrumentation::target_termination::insufficient_data);
            exit(0);
        }

        ++read_inputs;

        auto const type = (type_of_input_bits) bytes[cursor++];
        uint8_t const count = num_bytes(type);

        switch (type) {
            case type_of_input_bits::UINT8:
                *ptr = (T) *(uint8_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT8:
                *ptr = (T) *(int8_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT16:
                *ptr = (T) *(uint16_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT16:
                *ptr = (T) *(int16_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT32:
                *ptr = (T) *(uint32_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT32:
                *ptr = (T) *(int32_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::UINT64:
                *ptr = (T) *(uint64_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT64:
                *ptr = (T) *(int64_t*) (bytes.data() + cursor); break;
            case type_of_input_bits::FLOAT32:
                *ptr = (T) *(float*) (bytes.data() + cursor); break;
            case type_of_input_bits::FLOAT64:
                *ptr = (T) *(double*) (bytes.data() + cursor); break;
#if CPU_TYPE() == CPU64()
            case type_of_input_bits::UINT128:
                *ptr = (T) *(unsigned __int128*) (bytes.data() + cursor); break;
            case type_of_input_bits::SINT128:
                *ptr = (T) *(__int128*) (bytes.data() + cursor); break;
#endif
            default: ;
        }

        cursor += count;
    }


   private:
    uint64_t read_inputs = 0;
    uint64_t input_count;
    uint64_t m_max_bytes;
    uint64_t cursor;
    vecu8 bytes;
};

using input_model_ptr = std::unique_ptr<input_model>;

}  // namespace iomodels
