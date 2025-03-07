#include <driver/program_info.hpp>
#include <driver/program_options.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
//#if COMPILER() == COMPILER_VC()
//#   pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") 
//#endif

extern void run(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    initialise_program_options(argc,argv);
    if (get_program_options()->helpMode())
        std::cout << get_program_options();
    else if (get_program_options()->versionMode())
        std::cout << get_program_version() << "\n";
    else
    {
        run(argc,argv);
    }
    return 0;
}
