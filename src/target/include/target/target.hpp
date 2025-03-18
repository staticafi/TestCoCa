#pragma once

#include <connection/shared_memory.hpp>
#include <target/instrumentation_types.hpp>
#include <iomodels/configuration.hpp>
#include <iomodels/stdin_base.hpp>
#include <memory>
#include <utility/basic_numeric_types.hpp>

namespace instrumentation {

class target {
    std::mutex mutex;

   public:
    iomodels::configuration config;
    iomodels::stdin_base_ptr stdin_model;
    connection::shared_memory shared_memory;

    target();

    void process_br_instr(br_instr_id id, bool covered_branch);
    void process_ver_error();

    void on_read(natural_8_bit* ptr, type_of_input_bits type);

    void load_stdin();
    void load_config();
};

extern std::unique_ptr<target> target;

}  // namespace instrumentation
