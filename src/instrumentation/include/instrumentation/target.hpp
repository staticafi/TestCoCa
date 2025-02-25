#pragma once

#include <connection/shared_memory.hpp>
#include <instrumentation/instrumentation_types.hpp>
#include <iomodels/configuration.hpp>
#include <iomodels/stdin_base.hpp>
#include <iomodels/stdout_void.hpp>
#include <memory>
#include <utility/basic_numeric_types.hpp>

namespace instrumentation {

class fuzz_target {
   public:
    iomodels::stdin_base_ptr stdin_model;
    iomodels::stdout_base_ptr stdout_model;
    connection::shared_memory shared_memory;

    fuzz_target();

    void process_condition(location_id id, bool direction,
                           branching_function_value_type value,
                           bool xor_like_branching_function);
    void process_br_instr(location_id id, bool covered_branch);

    void process_call_begin(natural_32_bit const id);
    void process_call_end(natural_32_bit const id);

    void on_read(natural_8_bit* ptr, type_of_input_bits type);
    void on_write(natural_8_bit const* ptr, type_of_input_bits type);

    void load_stdin();
    void load_config();
};

extern std::unique_ptr<fuzz_target> sbt_fizzer_target;

}  // namespace instrumentation
