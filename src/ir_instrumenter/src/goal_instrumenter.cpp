#include <ir_instrumenter/goal_instrumenter.hpp>

#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <utility/timeprof.hpp>

#include <vector>

using namespace llvm;

namespace ir_instrumenter {

void goal_instrumenter::instrument(Module &M)
{
    TMPROF_BLOCK();

    std::vector<BasicBlock*> to_remove;

    for (Function &F: M) {
        if (F.isDeclaration()) continue;

        for (BasicBlock &BB: F) {
            if (&F.getEntryBlock() == &BB) continue;

            if (pred_empty(&BB)) {
                to_remove.push_back(&BB);
                continue;
            }

            for (Instruction &I : BB) {
                if (auto *call_inst = dyn_cast<CallInst>(&I)) {
                    Value *callee = call_inst->getCalledOperand()->stripPointerCasts();

                    StringRef calleeName;
                    if (auto *func = dyn_cast<Function>(callee)) {
                        calleeName = func->getName();
                    } else if (auto *gv = dyn_cast<GlobalValue>(callee)) {
                        calleeName = gv->getName();
                    }

                    if (calleeName == "__testcoca_process_goal") {
                        Value *new_arg = ConstantInt::get(Int32Ty, goalCounter);
                        call_inst->setArgOperand(0, new_arg);
                        goalCounter++;
                    }
                }
            }
        }
    }

    addGlobalCountConstant(goalCountName, goalCounter);

    for (auto *bb: to_remove) {
        DeleteDeadBlock(bb);
    }
}

}  // namespace ir_instrumenter
