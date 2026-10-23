#pragma once

#include <ir_instrumenter/llvm_instrumenter.hpp>

#include <llvm/IR/IRBuilder.h>

#include <string>

namespace ir_instrumenter {

// Inserts a call to __testcoca_process_ver_error() at the entry of the named
// function (which is looked up under the rename prefix), so that reaching an
// error function is observable.
class error_instrumenter : public llvm_instrumenter {
public:
    explicit error_instrumenter(std::string target_func);

    bool doInitialization(llvm::Module* M) override;

    void instrument(llvm::Module& M) override;

private:
    void instrumentVerifierError(llvm::Function* function) const;

    std::string target_func_;
    llvm::FunctionCallee processVerErrFunc;
};

}  // namespace ir_instrumenter
