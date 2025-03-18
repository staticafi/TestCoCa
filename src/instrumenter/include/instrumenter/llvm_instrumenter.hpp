#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/Passes/PassBuilder.h>

struct llvm_instrumenter {
    bool doInitialization(llvm::Module *M);

    void renameFunctions() const;

    bool runOnFunction(llvm::Function &F);

    void instrumentCondBr(llvm::BranchInst *brInst) const;

    void instrumentVerifierError(llvm::BranchInst *brInst) const;

    void addCondBrCount() const;

private:
    llvm::Module *module;

    llvm::IntegerType *Int32Ty;

    llvm::Type *VoidTy;

    llvm::FunctionCallee processCondBrFunc;
    llvm::FunctionCallee processVerErrFunc;

    unsigned int basicBlockCounter;
    unsigned int condBrCounter;
};
