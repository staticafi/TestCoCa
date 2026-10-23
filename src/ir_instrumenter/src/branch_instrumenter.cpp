#include <ir_instrumenter/branch_instrumenter.hpp>

#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <utility/timeprof.hpp>

using namespace llvm;

namespace ir_instrumenter {

bool branch_instrumenter::doInitialization(Module* M)
{
    llvm_instrumenter::doInitialization(M);

    processCondBrFunc = module->getOrInsertFunction("__testcoca_process_br_instr",
                                                    VoidTy, Int32Ty);
    return true;
}

void branch_instrumenter::instrumentCondBr(auto* brInst) const
{
    IRBuilder builder(brInst);

    Value* location = ConstantInt::get(Int32Ty, condBrCounter);
    Value* cond     = brInst->getCondition();

    builder.CreateCall(processCondBrFunc, {location, cond});
}

void branch_instrumenter::instrument(Module &M)
{
    TMPROF_BLOCK();

    for (Function &F: M) {

        if (F.isDeclaration()) continue;

        for (BasicBlock &BB: F) {
            auto *brInst = dyn_cast<BranchInst>(BB.getTerminator());
            if (brInst && brInst->isConditional()) {
                ++condBrCounter;
                instrumentCondBr(brInst);
            }

            for (auto &I: BB) {
                if (auto *selInst = dyn_cast<SelectInst>(&I)) {
                    ++condBrCounter;
                    instrumentCondBr(selInst);
                    break;
                }
            }
        }
    }

    addGlobalCountConstant(conditionalBrCountName, condBrCounter);
}

}  // namespace ir_instrumenter
