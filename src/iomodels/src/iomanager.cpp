#include <iomodels/iomanager.hpp>

#include "iomodels/stdin_replay_bytes_then_raise_error.hpp"

using namespace connection;
using namespace instrumentation;

namespace  iomodels {


iomanager::iomanager()
    : config{}
    , stdin_ptr(nullptr)
{}


iomanager&  iomanager::instance()
{
    static iomanager man;
    return man;
}


void  iomanager::set_config(configuration const&  cfg)
{
    config = cfg;
    config.invalidate_shared_memory_size_cache();
    stdin_ptr = nullptr;
}

stdin_base*  iomanager::get_stdin() const
{
    if (stdin_ptr == nullptr)
        stdin_ptr = std::make_unique<stdin_replay_bytes_then_raise_error>(100);
    return stdin_ptr.get();
}

}
