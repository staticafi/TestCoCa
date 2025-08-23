#pragma once

#include <map>
#include <connection/shared_memory.hpp>
#include <target/instrumentation_types.hpp>
#include <iomodels/configuration.hpp>
#include <memory>

#include "iomodels/input_model.hpp"

namespace instrumentation {

class target {
    std::mutex mutex;
    uint64_t index = 0;
    std::map<br_instr_id, uint64_t> index_map;

   public:
    iomodels::configuration config;
    iomodels::input_model_ptr stdin_model;
    connection::shared_memory shared_memory;

    target();

    void process_br_instr(br_instr_id id, condition_coverage covered_branch);
    void process_ver_error();
    void process_goal(goal_id id);

    void load_stdin();
    void load_config();

    template<typename T>
    void on_read(T* ptr)
    {
        std::lock_guard lock(mutex);
        stdin_model->read(ptr, shared_memory);
    }
};

extern std::unique_ptr<target> target;

}  // namespace instrumentation
