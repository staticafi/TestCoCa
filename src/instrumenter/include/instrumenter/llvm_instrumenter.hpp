#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <unordered_map>
#include <vector>

struct llvm_instrumenter {
    struct basic_block_dbg_info {
        static int constexpr invalid_depth = std::numeric_limits<int>::max();
        unsigned int id{0U};
        llvm::DILocation const *info{nullptr};
        int depth{invalid_depth};
    };

    using basic_block_dbg_info_map =
        std::unordered_map<llvm::BasicBlock const *, basic_block_dbg_info>;

    struct instruction_dbg_info {
        llvm::Instruction const *instruction{nullptr};
        unsigned int id{0U};
        unsigned int shift{0U};
    };

    using instruction_dbg_info_vector = std::vector<instruction_dbg_info>;

    bool doInitialization(llvm::Module *M);

    void renameFunctions() const;

    void replaceCalls(
        llvm::Function &F,
        std::unordered_map<std::string, llvm::FunctionCallee> replacements);

    bool runOnFunction(llvm::Function &F);

    static void printErrCond(llvm::Value *cond);

    void instrumentCondBr(llvm::BranchInst *brInst) const;

    void instrumentVerifierError(llvm::BranchInst *brInst) const;

    void propagateMissingBasicBlockDbgInfo();

    basic_block_dbg_info_map const &getBasicBlockDbgInfo() const
    {
        return basicBlockDbgInfo;
    }
    instruction_dbg_info_vector const &getCondInstrDbgInfo() const
    {
        return condInstrDbgInfo;
    }
    instruction_dbg_info_vector const &getBrInstrDbgInfo() const
    {
        return brInstrDbgInfo;
    }

   private:
    llvm::Module *module;

    llvm::IntegerType *Int1Ty;
    llvm::IntegerType *Int8Ty;
    llvm::IntegerType *Int16Ty;
    llvm::IntegerType *Int32Ty;
    llvm::IntegerType *Int64Ty;

    llvm::Type *VoidTy;

    llvm::Type *FloatTy;
    llvm::Type *DoubleTy;

    std::unique_ptr<llvm::legacy::FunctionPassManager> DependenciesFPM;

    llvm::FunctionCallee processCondBrFunc;
    llvm::FunctionCallee processVerErrFunc;

    unsigned int basicBlockCounter;
    unsigned int condCounter;
    unsigned int callSiteCounter;

    basic_block_dbg_info_map basicBlockDbgInfo;
    instruction_dbg_info_vector condInstrDbgInfo;
    instruction_dbg_info_vector brInstrDbgInfo;
};
