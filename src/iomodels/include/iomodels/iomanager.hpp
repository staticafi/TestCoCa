#pragma once

#   include <iomodels/configuration.hpp>
#   include <instrumentation/target_termination.hpp>
#   include <iomodels/stdin_base.hpp>
#   include <iomodels/stdout_base.hpp>
#   include <instrumentation/instrumentation_types.hpp>
#   include <vector>

namespace  iomodels {


struct  iomanager
{
    static iomanager&  instance();

    configuration const&  get_config() const { return config; }
    void  set_config(configuration const&  cfg);

    instrumentation::target_termination  get_termination() const { return termination; }

    void  load_results(connection::shared_memory& src);

    bool  load_br_instr_trace_record(connection::shared_memory& src);

    stdin_base*  get_stdin() const;

    stdout_base*  get_stdout() const;

private:
    iomanager();

    configuration config;
    instrumentation::target_termination  termination;
    mutable stdin_base_ptr  stdin_ptr;
    mutable stdout_base_ptr  stdout_ptr;
};


}