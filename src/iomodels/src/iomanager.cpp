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

}
