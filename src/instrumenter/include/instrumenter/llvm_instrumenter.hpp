#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/Passes/PassBuilder.h>

struct llvm_instrumenter {
    bool doInitialization(llvm::Module *M);

    void renameFunctions() const;

    bool runOnFunction(llvm::Function& F, bool instBr, bool instErr, std::string& targetFunc);

    void instrumentCondBr(auto *brInst) const;

    void instrumentVerifierError(llvm::Function *function) const;

    void addCondBrCount() const;

private:
    llvm::Module *module;

    llvm::IntegerType *Int32Ty;

    llvm::Type *VoidTy;

    llvm::FunctionCallee processCondBrFunc;
    llvm::FunctionCallee processVerErrFunc;

    std::string const renamePrefix{"__testcoca_rename_prefix__"};

    unsigned int basicBlockCounter;
    unsigned int condBrCounter;
};
