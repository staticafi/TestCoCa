#pragma once

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <optional>
#include <target/target_termination.hpp>
#include <utility/assumptions.hpp>
#include <utility/endian.hpp>
#include <iostream>

namespace connection {

class shared_memory {
    inline static const char* segment_name = "shared_memory";

    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region region;
    natural_32_bit cursor = 0;
    natural_8_bit* memory = nullptr;
    natural_32_bit* saved = nullptr;

   public:
    natural_32_bit get_size() const;
    void set_size(natural_32_bit bytes);
    void clear();
    void print();

    void open_or_create();
    void map_region();
    static void remove();

    bool can_accept_bytes(size_t n) const;
    bool can_deliver_bytes(size_t n) const;

    void accept_bytes(const void* src, size_t n);
    void deliver_bytes(void* dest, size_t n);

    bool exhausted() const;
    natural_8_bit* get_memory();

    std::optional<instrumentation::target_termination> get_termination() const;

    void set_termination(instrumentation::target_termination termination);

    natural_32_bit get_cond_br_count() const;

    template <typename T,
              typename std::enable_if<std::is_trivially_copyable<T>::value,
                                      int>::type = 0>
    shared_memory& operator<<(const T& src)
    {
        accept_bytes(&src, sizeof(T));
        return *this;
    }

    shared_memory& operator<<(const std::string& src);

    template <typename T,
              typename std::enable_if<std::is_trivially_copyable<T>::value,
                                      int>::type = 0>
    shared_memory& operator>>(T& dest)
    {
        deliver_bytes(&dest, sizeof(T));
        return *this;
    }

    shared_memory& operator>>(std::string& dest);
};

struct shared_memory_remover {
    ~shared_memory_remover()
    {
        shared_memory::remove();
    }
};

}  // namespace connection
