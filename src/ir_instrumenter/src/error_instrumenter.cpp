#include <ir_instrumenter/error_instrumenter.hpp>

#include <utility/timeprof.hpp>

#include <utility>

using namespace llvm;

namespace ir_instrumenter {

error_instrumenter::error_instrumenter(std::string target_func)
    : target_func_(std::move(target_func))
{
}

bool error_instrumenter::doInitialization(Module* M)
{
    llvm_instrumenter::doInitialization(M);

    processVerErrFunc = module->getOrInsertFunction("__testcoca_process_ver_error",
                                                    VoidTy);
    return true;
}

void error_instrumenter::instrumentVerifierError(Function* targetFunc) const
{
    BasicBlock &entryBB = targetFunc->getEntryBlock();

    auto firstNonPHI = entryBB.getFirstNonPHI();
    IRBuilder builder(&entryBB, firstNonPHI ? firstNonPHI->getIterator()
                                            : entryBB.begin());

    builder.CreateCall(processVerErrFunc);
}

void error_instrumenter::instrument(Module &M)
{
    TMPROF_BLOCK();

    for (Function &F: M) {

        if (F.isDeclaration()) continue;

        if (F.getName() == renamePrefix + target_func_) {
            instrumentVerifierError(&F);
        }
    }
}

}  // namespace ir_instrumenter
