#include <iomodels/iomanager.hpp>

#include "iomodels/input_model.hpp"

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

input_model* iomanager::get_stdin() const
{
    if (stdin_ptr == nullptr)
        stdin_ptr = std::make_unique<input_model>(1024*1024);
    return stdin_ptr.get();
}

}
