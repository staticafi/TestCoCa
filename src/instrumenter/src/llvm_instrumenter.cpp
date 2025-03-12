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

    Int1Ty  = IntegerType::getInt1Ty(C);
    Int8Ty  = IntegerType::getInt8Ty(C);
    Int32Ty = IntegerType::getInt32Ty(C);
    Int64Ty = IntegerType::getInt64Ty(C);

    VoidTy = Type::getVoidTy(C);

    FloatTy  = Type::getFloatTy(C);
    DoubleTy = Type::getDoubleTy(C);

    DependenciesFPM = std::make_unique<legacy::FunctionPassManager>(module);
    DependenciesFPM->add(createLowerSwitchPass());

    processCondBrFunc = module->getOrInsertFunction("__qmi_process_br_instr",
                                                    VoidTy, Int32Ty);

    processCondBrFunc = module->getOrInsertFunction("__qmi_process_ver_error",
                                                    VoidTy);

    basicBlockCounter = 0;

    return true;
}

void llvm_instrumenter::renameFunctions() const {
    TMPROF_BLOCK();

    std::string const renamePrefix{"__qmi_rename_prefix__"};
    for (auto it = module->begin(); it != module->end(); ++it) {
        Function& fn = *it;
        if (!fn.isDeclaration() && fn.getName() != "main")
            fn.setName(renamePrefix + fn.getName());
    }
}

void llvm_instrumenter::printErrCond(Value* cond)
{
    errs() << "Condition instruction is: ";
    cond->print(errs());
    errs() << "\n";
}

void llvm_instrumenter::instrumentCondBr(BranchInst* brInst) const {
    IRBuilder builder(brInst);

    Value* location = ConstantInt::get(Int32Ty, basicBlockCounter);
    Value* cond     = brInst->getCondition();

    builder.CreateCall(processCondBrFunc, {location, cond});
}

void llvm_instrumenter::instrumentVerifierError(BranchInst* brInst) const {
    IRBuilder builder(brInst);
    Value* location = ConstantInt::get(Int32Ty, basicBlockCounter);
    builder.CreateCall(processVerErrFunc, {location});
}

void llvm_instrumenter::replaceCalls(
    Function& F, std::unordered_map<std::string, FunctionCallee> replacements)
{
    TMPROF_BLOCK();

    std::vector<std::pair<CallInst*, FunctionCallee>> replaceCalls;

    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        if (auto* callInst = dyn_cast<CallInst>(&*I)) {
            Function* callee = callInst->getCalledFunction();
            if (!callee) {
                continue;
            }
            auto it = replacements.find(callee->getName().str());
            if (it != replacements.end()) {
                replaceCalls.emplace_back(callInst, it->second);
            }
        }
    }

    for (auto [callInst, replacement] : replaceCalls) {
        ReplaceInstWithInst(callInst, CallInst::Create(replacement));
    }
}

bool llvm_instrumenter::runOnFunction(Function& F)
{
    TMPROF_BLOCK();

    if (F.isDeclaration()) {
        return false;
    }

    DependenciesFPM->run(F);

    if (F.getName() == "main") {
        F.setName("__qmi_original_main");
    }

    for (BasicBlock& BB : F) {
        ++basicBlockCounter;
        BB.setName("bb" + std::to_string(basicBlockCounter));

        basic_block_dbg_info& bbDbgInfo = basicBlockDbgInfo[&BB];
        bbDbgInfo.id                    = basicBlockCounter;

        BranchInst* brInst = dyn_cast<BranchInst>(BB.getTerminator());
        if (!brInst || !brInst->isConditional()) {
            continue;
        }

        instrumentCondBr(brInst);

        brInstrDbgInfo.push_back(
            {brInst, basicBlockCounter, (unsigned int)BB.size()});
    }
    return true;
}

void llvm_instrumenter::propagateMissingBasicBlockDbgInfo()
{
    TMPROF_BLOCK();

    struct local {
        using key_type = std::pair<llvm::DILocation const*, int>;

        static bool less_than(key_type const& key0, key_type const& key1)
        {
            if (key0.first != nullptr && key1.first == nullptr) return true;
            if (key0.first == nullptr) return false;

            if (key0.second < key1.second) return true;
            if (key0.second != key1.second) return false;

            if (key0.first->getLine() < key1.first->getLine()) return true;
            if (key0.first->getLine() != key1.first->getLine()) return false;

            if (key0.first->getColumn() < key1.first->getColumn()) return true;
            return false;
        }

        static void compute_basic_block_dbg_info(
            llvm::BasicBlock const* const bb,
            llvm_instrumenter::basic_block_dbg_info_map& bbInfo,
            std::unordered_set<llvm::BasicBlock const*>& visited)
        {
            llvm_instrumenter::basic_block_dbg_info& info = bbInfo.at(bb);
            if (info.depth != basic_block_dbg_info::invalid_depth) return;

            if (visited.contains(bb)) {
                info.depth = 0;
                return;
            }

            visited.insert(bb);

            llvm::BranchInst const* const brInstr =
                llvm::dyn_cast<llvm::BranchInst>(bb->getTerminator());
            if (brInstr == nullptr) {
                info.depth = 0;
                return;
            }

            local::key_type best{info.info, info.depth};
            for (unsigned int i = 0; i < brInstr->getNumSuccessors(); ++i) {
                compute_basic_block_dbg_info(brInstr->getSuccessor(i), bbInfo,
                                             visited);
                llvm_instrumenter::basic_block_dbg_info& succ_info =
                    bbInfo.at(brInstr->getSuccessor(i));
                local::key_type const succ_key{succ_info.info,
                                               succ_info.depth + 1};
                if (local::less_than(succ_key, best)) best = succ_key;
            }

            info.info  = best.first;
            info.depth = best.second;
        }
    };

    std::unordered_set<llvm::BasicBlock const*> visited{};

    for (auto const& info : condInstrDbgInfo)
        if (info.instruction->getDebugLoc().get() == nullptr)
            local::compute_basic_block_dbg_info(info.instruction->getParent(),
                                                basicBlockDbgInfo, visited);
    for (auto const& info : brInstrDbgInfo)
        if (info.instruction->getDebugLoc().get() == nullptr)
            local::compute_basic_block_dbg_info(info.instruction->getParent(),
                                                basicBlockDbgInfo, visited);
}
