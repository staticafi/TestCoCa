#pragma once

#include <ir_instrumenter/llvm_instrumenter.hpp>

#include <cstdint>
#include <string>

namespace ir_instrumenter {

// Renumbers the __testcoca_process_goal() calls emitted by a source-level
// instrumenter (label-adder or the AST instrumenter), removes unreachable basic
// blocks, and emits the __testcoca_goal_count global.
class goal_instrumenter : public llvm_instrumenter {
public:
    void instrument(llvm::Module& M) override;

private:
    uint32_t goalCounter = 0;
    std::string const goalCountName = "__testcoca_goal_count";
};

}  // namespace ir_instrumenter
