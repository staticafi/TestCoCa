#pragma once

#include <iomodels/configuration.hpp>
#include <target/target_termination.hpp>
#include <iomodels/stdin_base.hpp>

namespace  iomodels {


struct  iomanager
{
    static iomanager&  instance();

    configuration const&  get_config() const { return config; }
    void  set_config(configuration const&  cfg);

    instrumentation::target_termination  get_termination() const { return termination; }

    void  load_results(connection::shared_memory& src);

    stdin_base*  get_stdin() const;

private:
    iomanager();

    configuration config;
    instrumentation::target_termination  termination;
    mutable stdin_base_ptr  stdin_ptr;
};


}