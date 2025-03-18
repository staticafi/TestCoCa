#include <iomanip>
#include <connection/shared_memory.hpp>
#include <iomodels/configuration.hpp>
#include <iostream>

namespace bip = boost::interprocess;
using namespace instrumentation;

namespace connection {

void shared_memory::open_or_create()
{
    shm = bip::shared_memory_object(bip::open_or_create, segment_name,
                                    bip::read_write);
}

natural_32_bit shared_memory::get_size() const
{
    size_t size = region.get_size();
    if (size != 0) {
        return (natural_32_bit)(size - sizeof(*saved));
    }
    return (natural_32_bit)size;
}

void shared_memory::set_size(natural_32_bit size)
{
    shm.truncate(size + sizeof(*saved));
}

void shared_memory::clear()
{
    memset(memory, 0, region.get_size()); //TODO remove, only for testing
    cursor = 0;
    *saved = 0;
}

void shared_memory::map_region()
{
    region = bip::mapped_region(shm, bip::read_write);
    cursor = 0;
    saved  = static_cast<natural_32_bit*>(region.get_address());
    memory = static_cast<natural_8_bit*>(region.get_address()) + sizeof(*saved);
}

void shared_memory::remove()
{
    bip::shared_memory_object::remove(segment_name);
}

bool shared_memory::can_accept_bytes(size_t const n) const
{
    return !(memory == nullptr || get_size() < *saved + n);
}

bool shared_memory::can_deliver_bytes(size_t const n) const
{
    return !(memory == nullptr || *saved < cursor + n);
}

void shared_memory::accept_bytes(const void* src, size_t n)
{
    memcpy(memory + *saved, src, n);
    *saved += (natural_32_bit)n;
}

void shared_memory::deliver_bytes(void* dest, size_t n)
{
    memcpy(dest, memory + cursor, n);
    cursor += (natural_32_bit)n;
}

shared_memory& shared_memory::operator<<(const std::string& src)
{
    *this << (natural_32_bit)src.size();
    accept_bytes(src.data(), (natural_32_bit)src.size());
    return *this;
}

shared_memory& shared_memory::operator>>(std::string& dest)
{
    natural_32_bit size;
    *this >> size;
    dest.resize(size);
    deliver_bytes(dest.data(), (size));
    return *this;
}

std::optional<target_termination> shared_memory::get_termination() const
{
    target_termination termination =
        static_cast<target_termination>(*memory);
    if (!valid_termination(termination)) {
        return std::nullopt;
    }

    return termination;
}

natural_32_bit shared_memory::get_cond_br_count() const {
    return *(memory + 2);
}

void shared_memory::set_termination(target_termination termination)
{
    *memory = static_cast<natural_8_bit>(termination);
}

bool shared_memory::exhausted() const
{
    return cursor >= *saved;
}


natural_8_bit* shared_memory::get_memory(){
    return memory;
}

void shared_memory::print() {
    auto buffer = memory;
    int size = get_size();

    std::cout << "Shared Memory Dump (" << size << " bytes):\n";

    for (size_t i = 0; i < size; i += 16) {  // Print 16 bytes per row
        std::cout << std::setw(4) << std::setfill('0') << i << "  ";

        // Print hex values
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (int)buffer[i + j] << " " << std::dec;
            } else {
                std::cout << "   ";  // Padding for alignment
            }
        }

        std::cout << " ";

        // Print ASCII characters
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                char c = buffer[i + j];
                std::cout << (std::isprint(c) ? c : '.');  // Replace non-printable chars
            }
        }

        std::cout << std::endl;
    }
}

}  // namespace connection
