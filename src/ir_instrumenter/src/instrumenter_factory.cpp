#include <ir_instrumenter/instrumenter_factory.hpp>

#include <ir_instrumenter/branch_instrumenter.hpp>
#include <ir_instrumenter/error_instrumenter.hpp>
#include <ir_instrumenter/goal_instrumenter.hpp>
#include <ir_instrumenter/rename_only_instrumenter.hpp>

#include <iostream>
#include <string>
#include <utility>

namespace ir_instrumenter {

std::unique_ptr<llvm_instrumenter> create_instrumenter(const program_options_ptr& options)
{
    const bool instBr = options->has("inst_br");
    const bool instGoals = options->has("inst_goals");
    const bool instErr = options->has("inst_err");

    if ((int)instBr + (int)instGoals + (int)instErr > 1) {
        std::cout << "Only one of --inst_br, --inst_goals and --inst_err may be "
                     "specified."
                  << std::endl;
        return nullptr;
    }

    if (instBr) {
        return std::make_unique<branch_instrumenter>();
    }
    if (instGoals) {
        return std::make_unique<goal_instrumenter>();
    }
    if (instErr) {
        std::string target_func = options->value("inst_err");
        if (target_func.empty()) {
            std::cout << "InstErr option picked but no function specified"
                      << std::endl;
            return nullptr;
        }
        return std::make_unique<error_instrumenter>(std::move(target_func));
    }

    return std::make_unique<rename_only_instrumenter>();
}

}  // namespace ir_instrumenter
