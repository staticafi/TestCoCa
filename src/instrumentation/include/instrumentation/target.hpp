#pragma once

#include <connection/shared_memory.hpp>
#include <instrumentation/instrumentation_types.hpp>
#include <iomodels/configuration.hpp>
#include <iomodels/stdin_base.hpp>
#include <iomodels/stdout_void.hpp>
#include <memory>
#include <utility/basic_numeric_types.hpp>

namespace instrumentation {

class target {
   public:
    iomodels::configuration config;
    iomodels::stdin_base_ptr stdin_model;
    iomodels::stdout_base_ptr stdout_model;
    connection::shared_memory shared_memory;

    target();

    void process_br_instr(location_id id, bool covered_branch);
    void process_ver_error();

    void on_read(natural_8_bit* ptr, type_of_input_bits type);
    void on_write(natural_8_bit const* ptr, type_of_input_bits type);

    void load_stdin();
    void load_stdout();
    void load_config();
};

extern std::unique_ptr<target> target;

}  // namespace instrumentation
