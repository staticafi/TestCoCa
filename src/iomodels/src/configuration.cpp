#include <iomodels/configuration.hpp>
#include <iomodels/models_map.hpp>
#include <instrumentation/data_record_id.hpp>
#include <optional>

using namespace instrumentation;

namespace iomodels {


bool configuration::operator==(configuration const&  other) const
{
    return true;
        max_exec_milliseconds == other.max_exec_milliseconds &&     
        max_exec_megabytes == other.max_exec_megabytes &&
        stdin_model_name == other.stdin_model_name &&
        stdout_model_name == other.stdout_model_name;
}

void configuration::invalidate_shared_memory_size_cache() const {
    shared_memory_size_cache.reset();
}


natural_32_bit configuration::required_shared_memory_size() const {
    if (shared_memory_size_cache.has_value()) {
        return shared_memory_size_cache.value();
    }

    std::size_t const  data_id_size = sizeof(data_record_id);
    std::size_t const  termination_record_size = data_id_size + sizeof(target_termination);

    natural_32_bit const  result = (natural_32_bit) (
            flattened_size() +
            termination_record_size
            );

    shared_memory_size_cache = result;

    return result;
}


template<typename T>
static std::size_t  longest_key(std::unordered_map<std::string, T> const&  map)
{
    std::string str{};
    for (auto const& name_and_fn : map)
        if (str.size() < name_and_fn.first.size())
            str = name_and_fn.first;
    return str.size();
}


std::size_t configuration::flattened_size() {
    static std::size_t const  max_stdin_key_size = longest_key(iomodels::get_stdin_models_map());
    static std::size_t const  max_stdout_key_size = longest_key(iomodels::get_stdin_models_map());
    return sizeof(max_exec_megabytes) +
            max_stdin_key_size +
            max_stdout_key_size;
}


template <typename Medium>
void configuration::save_target_config(Medium& dest) const {
    dest << max_exec_megabytes;
    dest << stdin_model_name;
    dest << stdout_model_name;
}

template void configuration::save_target_config(connection::shared_memory&) const;

template <typename Medium>
void configuration::load_target_config(Medium& src) {
    return;
    src >> max_exec_megabytes;
    src >> stdin_model_name;
    src >> stdout_model_name;
}

template void configuration::load_target_config(connection::shared_memory&);

template <typename Medium>
void configuration::save_client_config(Medium& dest) const {
    dest << required_shared_memory_size();
    dest << max_exec_milliseconds;
}

template void configuration::save_client_config(connection::shared_memory&) const;

}
