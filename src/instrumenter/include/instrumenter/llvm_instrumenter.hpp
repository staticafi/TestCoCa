#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/Passes/PassBuilder.h>

struct llvm_instrumenter {
    bool doInitialization(llvm::Module *M);

    void renameFunctions() const;

    bool runOnFunction(llvm::Function& F, bool remove_unreachable, bool instBr, bool instErr, const std::string& targetFunc);

    void instrument_br(llvm::Module& M);

    void instrument_goals(llvm::Module& M);

    void instrument_err(llvm::Module& M, const std::string& name) const;

    void instrumentCondBr(auto *brInst) const;

    void instrumentVerifierError(llvm::Function *function) const;

    void addGlobalCountConstant(const std::string& name, uint32_t count) const;

    std::vector<llvm::BasicBlock*> deadBBs;
private:
    llvm::Module *module;

    llvm::IntegerType *Int32Ty;
    llvm::Type *VoidTy;

    llvm::FunctionCallee processCondBrFunc;
    llvm::FunctionCallee processVerErrFunc;

    std::string const renamePrefix{"__testcoca_rename_prefix__"};
    std::string const conditionalBrCountName = "__testcoca_cond_br_count";
    std::string const goalCountName = "__testcoca_goal_count";

    uint32_t condBrCounter = 0;
    uint32_t goalCounter = 0;
};
