#include <ir_instrumenter/llvm_instrumenter.hpp>

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/Alignment.h>
#include <utility/timeprof.hpp>

using namespace llvm;

namespace ir_instrumenter {

bool llvm_instrumenter::doInitialization(Module* M)
{
    TMPROF_BLOCK();

    module = M;

    LLVMContext& C = module->getContext();

    Int32Ty = IntegerType::getInt32Ty(C);
    VoidTy = Type::getVoidTy(C);

    return true;
}

void llvm_instrumenter::renameFunctions() const
{
    TMPROF_BLOCK();

    for (auto &fn: *module) {
        if (!fn.isDeclaration() && fn.getName() != "main") {
            fn.setName(renamePrefix + fn.getName());
        } else if (fn.getName() == "main") {
            fn.setName("__testcoca_original_main");
        }
    }
}

void llvm_instrumenter::addGlobalCountConstant(const std::string& name, uint32_t count) const
{
    Constant *initializer = ConstantInt::get(Int32Ty, count);
    GlobalVariable* count_global = new GlobalVariable(*module,
        Int32Ty,
        true,
        GlobalValue::ExternalLinkage,
        initializer,
        name);

    count_global->setAlignment(Align(4));
}

}  // namespace ir_instrumenter
