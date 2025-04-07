#include <iostream>
#include <llvm/IR/InstIterator.h>

#include <unordered_set>
#include <instrumenter/llvm_instrumenter.hpp>
#include <utility/timeprof.hpp>

#include "instrumenter/program_options.hpp"

using namespace llvm;

bool llvm_instrumenter::doInitialization(Module* M)
{
    TMPROF_BLOCK();

    module = M;

    LLVMContext& C = module->getContext();

    Int32Ty = IntegerType::getInt32Ty(C);
    VoidTy = Type::getVoidTy(C);

    processCondBrFunc = module->getOrInsertFunction("__qmi_process_br_instr",
                                                    VoidTy, Int32Ty);

    processVerErrFunc = module->getOrInsertFunction("__qmi_process_ver_error",
                                                    VoidTy);

    basicBlockCounter = 0;
    condBrCounter = 0;


    return true;
}

void llvm_instrumenter::renameFunctions() const {
    TMPROF_BLOCK();

    for (auto & fn : *module) {
        if (!fn.isDeclaration() && fn.getName() != "main")
            fn.setName(renamePrefix + fn.getName());
    }
}

void llvm_instrumenter::instrumentCondBr(BranchInst* brInst) const {
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

void llvm_instrumenter::addCondBrCount() const {
    Constant *initializer = ConstantInt::get(Int32Ty, condBrCounter);
    GlobalVariable* br_count = new GlobalVariable(*module,
        Int32Ty,
        true,
        GlobalValue::ExternalLinkage,
        initializer,
        "__qmi_cond_br_count");

    br_count->setAlignment(Align(4));
}

bool llvm_instrumenter::runOnFunction(Function& F, bool instBr, bool instErr, std::string& targetFunc)
{
    TMPROF_BLOCK();

    if (F.isDeclaration()) {
        return false;
    }

    if (F.getName() == "main") {
        F.setName("__qmi_original_main");
    }

    for (BasicBlock& BB : F) {
        ++basicBlockCounter;
        BB.setName("bb" + std::to_string(basicBlockCounter));

        if (instErr) {
            if (F.getName() ==  renamePrefix + targetFunc) {
                instrumentVerifierError(&F);
            }
        }

        if (instBr) {
            auto* brInst = dyn_cast<BranchInst>(BB.getTerminator());
            if (!brInst || !brInst->isConditional()) {
                continue;
            }

            ++condBrCounter;
            instrumentCondBr(brInst);
        }

    }
    return true;
}
