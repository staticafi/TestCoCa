#include <instrumenter/program_info.hpp>
#include <instrumenter/program_options.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>
#include <fstream>
#include <iostream>

extern void run(int argc, char* argv[]);

#if BUILD_RELEASE() == 1
static void save_crash_report(std::string const& crash_message)
{
    std::cerr << "ERROR: " << crash_message << "\n";
    std::ofstream  ofile( get_program_name() + "_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}
#endif

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
