#include <iomodels/configuration.hpp>
#include <optional>

using namespace instrumentation;

namespace iomodels {
    bool configuration::operator==(configuration const&  other) const
    {
        return max_exec_milliseconds == other.max_exec_milliseconds &&
        max_exec_megabytes == other.max_exec_megabytes &&
        stdin_model_name == other.stdin_model_name;
    }

    void configuration::invalidate_shared_memory_size_cache() const {
        shared_memory_size_cache.reset();
    }


    natural_32_bit configuration::required_shared_memory_size() const {
        if (shared_memory_size_cache.has_value()) {
            return shared_memory_size_cache.value();
        }

        std::size_t const  termination_record_size = sizeof(target_termination);

        natural_32_bit const  result = (natural_32_bit) (
                flattened_size() +
                termination_record_size
                );

        shared_memory_size_cache = result;

        return result;
    }


    std::size_t configuration::flattened_size() {
        /*
        static std::size_t const  max_stdin_key_size = longest_key(iomodels::get_stdin_models_map());
        static std::size_t const  max_stdout_key_size = longest_key(iomodels::get_stdin_models_map());
        return sizeof(max_exec_megabytes) +
                max_stdin_key_size +
                max_stdout_key_size;
                */
    }


    void configuration::save_target_config(connection::shared_memory& dest) const {
        dest << max_exec_megabytes;
        dest << stdin_model_name;
    }

    void configuration::load_target_config(connection::shared_memory& src) {
        return;
        src >> max_exec_megabytes;
        src >> stdin_model_name;
    }

    void configuration::save_client_config(connection::shared_memory& dest) const {
        dest << required_shared_memory_size();
        dest << max_exec_milliseconds;
    }
}