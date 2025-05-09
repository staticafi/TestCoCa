#pragma once

#include <iomodels/configuration.hpp>
#include <target/target_termination.hpp>

#include "input_model.hpp"

namespace  iomodels {


struct  iomanager
{
    static iomanager&  instance();

    configuration const&  get_config() const { return config; }
    void  set_config(configuration const&  cfg);

    instrumentation::target_termination  get_termination() const { return termination; }

private:
    iomanager();

    configuration config;
    instrumentation::target_termination  termination;
    mutable input_model_ptr stdin_ptr;
};


}
