#include <ast_instrumenter/condition_instrumenter.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.c> <output.c>\n";
        return 1;
    }

    std::vector<std::string> compiler_args = {"-std=gnu11", "-Wno-everything"};

    if (!ast_instrumenter::instrument_conditions(argv[1], argv[2],
                                                 compiler_args)) {
        std::cerr << "ast_instrumenter: failed to process '" << argv[1]
                  << "'\n";
        return 1;
    }
    return 0;
}
