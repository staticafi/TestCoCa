#pragma once

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <cstdint>
#include <optional>
#include <target/target_termination.hpp>
#include <utility/assumptions.hpp>
#include <utility/endian.hpp>

namespace connection {

class shared_memory {
    inline static const char* segment_name = "shared_memory";

    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region region;

    natural_64_bit cursor = 0;
    natural_8_bit* memory = nullptr;
    natural_64_bit* saved = nullptr;

   public:
    struct metadata_header {
        uint32_t cond_br_count;
        uint32_t goal_count;
        uint64_t header_checksum;
        uint64_t data_checksum;
        instrumentation::target_termination termination;
        uint8_t _pad[7];

        void set_header_checksum()
        {
            header_checksum = (uint64_t)cond_br_count ^ ((uint64_t)goal_count) << 32;
        }
        
        bool verify_header_checksum()
        {
            return header_checksum == ((uint64_t)cond_br_count ^ ((uint64_t)goal_count) << 32);
        }
    };


    size_t get_size() const;
    void set_size(size_t bytes);
    void clear();
    void print(size_t size = 0);

    void open_or_create();
    void map_region();
    static void remove();

    bool can_accept_bytes(size_t n) const;
    bool can_deliver_bytes(size_t n) const;

    void accept_bytes(const void* src, size_t n) const;
    void deliver_bytes(void* dest, size_t n);

    bool exhausted() const;
    natural_8_bit* get_memory();

    metadata_header* header() const;
    natural_8_bit* coverage_ptr() const;
    natural_64_bit* saved_cursor() const { return saved; }

    std::optional<instrumentation::target_termination> get_termination() const;

    void set_termination(instrumentation::target_termination termination);

    uint32_t get_cond_br_count() const;
    uint32_t get_goal_count() const;

    uint64_t *checksum() const;

    template <typename T,
              typename std::enable_if<std::is_trivially_copyable<T>::value,
                                      int>::type = 0>
    shared_memory& operator<<(const T& src)
    {
        accept_bytes(&src, sizeof(T));
        return *this;
    }

    template <typename T,
              typename std::enable_if<std::is_trivially_copyable<T>::value,
                                      int>::type = 0>
    shared_memory& operator>>(T& dest)
    {
        deliver_bytes(&dest, sizeof(T));
        return *this;
    }
};

struct shared_memory_remover {
    ~shared_memory_remover()
    {
        shared_memory::remove();
    }
};

}  // namespace connection
