#pragma once

#include <ir_instrumenter/llvm_instrumenter.hpp>

#include <llvm/IR/IRBuilder.h>

#include <cstdint>
#include <string>

namespace ir_instrumenter {

// Instruments every conditional branch and every select instruction so that the
// runtime can record which edge was taken. Emits the __testcoca_cond_br_count
// global.
class branch_instrumenter : public llvm_instrumenter {
public:
    bool doInitialization(llvm::Module* M) override;

    void instrument(llvm::Module& M) override;

private:
    void instrumentCondBr(auto* brInst) const;

    llvm::FunctionCallee processCondBrFunc;

    uint32_t condBrCounter = 0;
    std::string const conditionalBrCountName = "__testcoca_cond_br_count";
};

}  // namespace ir_instrumenter
