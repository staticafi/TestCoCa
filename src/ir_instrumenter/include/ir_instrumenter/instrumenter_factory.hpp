#pragma once

#include <ir_instrumenter/llvm_instrumenter.hpp>
#include <ir_instrumenter/program_options.hpp>

#include <memory>

namespace ir_instrumenter {

// Picks the single instrumenter described by the command line. Exactly one of
// --inst_br, --inst_goals, --inst_err may be given; if none is given, a
// rename-only instrumenter is returned. Returns nullptr on an invalid
// combination (more than one capability, or --inst_err without a function).
std::unique_ptr<llvm_instrumenter> create_instrumenter(const program_options_ptr& options);

}  // namespace ir_instrumenter
