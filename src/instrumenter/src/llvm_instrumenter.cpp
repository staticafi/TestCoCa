#include <llvm/IR/InstIterator.h>

#include <unordered_set>
#include <instrumenter/llvm_instrumenter.hpp>
#include <utility/timeprof.hpp>

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

    std::string const renamePrefix{"__qmi_rename_prefix__"};
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

void llvm_instrumenter::instrumentVerifierError(BranchInst* brInst) const {
    IRBuilder builder(brInst);
    Value* location = ConstantInt::get(Int32Ty, basicBlockCounter);
    builder.CreateCall(processVerErrFunc, {location});
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

bool llvm_instrumenter::runOnFunction(Function& F)
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

        auto* brInst = dyn_cast<BranchInst>(BB.getTerminator());
        if (!brInst || !brInst->isConditional()) {
            continue;
        }

        ++condBrCounter;
        instrumentCondBr(brInst);

    }
    return true;
}
