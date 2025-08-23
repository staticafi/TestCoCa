#include <driver/program_options.hpp>
#include <driver/program_info.hpp>
#include <iomodels/iomanager.hpp>
#include <utility/assumptions.hpp>

program_options::program_options(int argc, char* argv[])
    : program_options_default(argc, argv)
{
    add_option("output_dir", "A directory where to store generated tests.", "1");

    add_option("clear_output_dir", "Erase the content of the output directory, if there is some.", "0");

    iomodels::configuration const  io_cfg{};

    add_option("goal", "Test type the target executable is instrumented for. Currently 'call' or 'coverage'", "1");
    add_value("goal", "coverage");

    add_option("testcomp", "Coverage will be calculated label-adder script", "0");

    add_option("max_exec_milliseconds", "Max number of milliseconds for benchmark execution.", "1");
    add_value("max_exec_milliseconds", std::to_string(io_cfg.max_exec_milliseconds));

    add_option("max_exec_megabytes", "Max number of mega bytes which can be allocated during benchmark execution.", "1");
    add_value("max_exec_megabytes", std::to_string(io_cfg.max_exec_megabytes));

    add_option("path_to_target", "Path to target executable.", "1");
    add_option("test_dir", "Path to the directory containing tests", "1");

    add_option("silent_mode", "Reduce the amount of messages printed to stdout.", "0");
}

static program_options_ptr  global_program_options;

void initialise_program_options(int argc, char* argv[])
{
    ASSUMPTION(!global_program_options.operator bool());
    global_program_options = program_options_ptr(new program_options(argc,argv));
}

program_options_ptr get_program_options()
{
    ASSUMPTION(global_program_options.operator bool());
    return global_program_options;
}

std::ostream& operator<<(std::ostream& ostr, program_options_ptr const& options)
{
    ASSUMPTION(options.operator bool());
    options->operator<<(ostr);
    return ostr;
}
