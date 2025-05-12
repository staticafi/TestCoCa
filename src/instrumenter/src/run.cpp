#include <llvm/DebugInfo/DIContext.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/IR/Function.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_os_ostream.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <instrumenter/llvm_instrumenter.hpp>
#include <memory>
#include <instrumenter/program_options.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>

void run(int argc, char* argv[])
{
    TMPROF_BLOCK();

    if (get_program_options()->has("help")) {
        std::cout << get_program_options() << std::endl;
        return;
    }
    if (get_program_options()->has("version")) {
        std::cout << get_program_options()->value("version") << std::endl;
        return;
    }
    if (!get_program_options()->has("input")) {
        std::cout << "No input file was specified." << std::endl;
        return;
    }
    if (!std::filesystem::is_regular_file(
            get_program_options()->value("input"))) {
        std::cout << "Cannot access the input file: "
                  << get_program_options()->value("input") << std::endl;
        return;
    }
    if (!get_program_options()->has("output")) {
        std::cout << "No output file was specified." << std::endl;
        return;
    }

    bool instBr = get_program_options()->has("inst_br"); bool instErr = get_program_options()->has("inst_err");

    if (!instBr && !instErr) {
        std::cout << "No instrumentation type selected (--inst_br or --inst_err <func>)" << std::endl;
        return;
    }

    std::string targetFunc;

    if (instErr && (targetFunc = get_program_options()->value("inst_err")).empty()) {
        std::cout << "InstErr option picked but no function specified" << std::endl;
        return;
    }

    llvm::SMDiagnostic D;
    llvm::LLVMContext C;
    std::unique_ptr<llvm::Module> M;
    {
        TMPROF_BLOCK();

        M = parseIRFile(get_program_options()->value("input"), D, C);
        if (M == nullptr) {
            llvm::raw_os_ostream ros(std::cout);
            D.print(std::filesystem::path(get_program_options()->value("input"))
                        .filename()
                        .string()
                        .c_str(),
                    ros, false);
            ros.flush();
            return;
        }
    }

    llvm::legacy::PassManager passManager;

    // remove switch statements from llvm IR
    passManager.add(llvm::createLowerSwitchPass());

    llvm::SimplifyCFGOptions Options;
    Options.BonusInstThreshold = 0;
    // force all selects to branches
    passManager.add(llvm::createCFGSimplificationPass(Options));

    passManager.run(*M);

    llvm_instrumenter instrumenter;
    instrumenter.doInitialization(M.get());
    instrumenter.renameFunctions();

    for (auto & it : *M) {
        instrumenter.runOnFunction(it, instBr, instErr,  targetFunc);
    }

    instrumenter.addCondBrCount();

    {
        TMPROF_BLOCK();

        std::ofstream ostr(get_program_options()->value("output").c_str(),
                           std::ios::binary);
        llvm::raw_os_ostream ros(ostr);
        M->print(ros, 0);
        ros.flush();
    }
}
