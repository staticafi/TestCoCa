#include <driver/program_info.hpp>
#include <driver/program_options.hpp>
#include <driver/test_runner.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    initialise_program_options(argc,argv);
    if (get_program_options()->helpMode())
        std::cout << get_program_options();
    else if (get_program_options()->versionMode())
        std::cout << get_program_version() << "\n";
    else
    {
        test_runner(get_program_options()).run();
    }
    return 0;
}
