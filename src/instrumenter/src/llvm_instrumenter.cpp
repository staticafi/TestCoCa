#include <iostream>
#include <llvm/IR/InstIterator.h>

#include <instrumenter/llvm_instrumenter.hpp>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <utility/timeprof.hpp>

using namespace llvm;

bool llvm_instrumenter::doInitialization(Module* M)
{
    TMPROF_BLOCK();

    module = M;

    LLVMContext& C = module->getContext();

    Int32Ty = IntegerType::getInt32Ty(C);
    VoidTy = Type::getVoidTy(C);

    processCondBrFunc = module->getOrInsertFunction("__testcoca_process_br_instr",
                                                    VoidTy, Int32Ty);

    processVerErrFunc = module->getOrInsertFunction("__testcoca_process_ver_error",
                                                    VoidTy);
    return true;
}

void llvm_instrumenter::renameFunctions() const {
    TMPROF_BLOCK();

    for (auto &fn: *module) {
        if (!fn.isDeclaration() && fn.getName() != "main") {
            fn.setName(renamePrefix + fn.getName());
        } else if (fn.getName() == "main") {
            fn.setName("__testcoca_original_main");
        }
    }
}

void llvm_instrumenter::instrumentCondBr(auto* brInst) const {
    IRBuilder builder(brInst);

    Value* location = ConstantInt::get(Int32Ty, condBrCounter);
    Value* cond     = brInst->getCondition();

    builder.CreateCall(processCondBrFunc, {location, cond});
}

void llvm_instrumenter::instrumentVerifierError(Function* targetFunc) const {
    BasicBlock &entryBB = targetFunc->getEntryBlock();

    auto firstNonPHI = entryBB.getFirstNonPHI();
    IRBuilder builder(&entryBB, firstNonPHI ? firstNonPHI->getIterator()
                                            : entryBB.begin());

    builder.CreateCall(processVerErrFunc);
}

void llvm_instrumenter::addGlobalCountConstant(const std::string& name, uint32_t count) const {
    Constant *initializer = ConstantInt::get(Int32Ty, count);
    GlobalVariable* br_count = new GlobalVariable(*module,
        Int32Ty,
        true,
        GlobalValue::ExternalLinkage,
        initializer,
        name);

    br_count->setAlignment(Align(4));
}

bool llvm_instrumenter::runOnFunction(Function& F, const bool remove_unreachable, const bool instBr, const bool instErr, const std::string& targetFunc)
{
    TMPROF_BLOCK();

    if (F.isDeclaration()) {
        return false;
    }

    if (F.getName() == "main") {
        F.setName("__testcoca_original_main");
    }

    for (BasicBlock& BB : F) {
    }

    return true;
}

void llvm_instrumenter::instrument_err(Module &M, const std::string& name) const {
    for (Function &F: M) {

        if (F.isDeclaration()) continue;

        if (F.getName() ==  renamePrefix + name) {
            instrumentVerifierError(&F);
        }
    }
}

void llvm_instrumenter::instrument_br(Module &M) {
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

void llvm_instrumenter::instrument_goals(Module &M) {
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