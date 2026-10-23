#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <cstdint>
#include <string>

namespace ir_instrumenter {

// Base class for every IR-level instrumenter. It owns the lifecycle shared by
// all of them: caching the module and the types used across the module, the
// function renaming scheme, and the emission of count globals. Each concrete
// instrumenter implements exactly one kind of instrumentation in instrument().
class llvm_instrumenter {
public:
    virtual ~llvm_instrumenter() = default;

    virtual bool doInitialization(llvm::Module* M);

    void renameFunctions() const;

    virtual void instrument(llvm::Module& M) = 0;

protected:
    void addGlobalCountConstant(const std::string& name, uint32_t count) const;

    llvm::Module* module = nullptr;

    llvm::IntegerType* Int32Ty = nullptr;
    llvm::Type* VoidTy = nullptr;

    std::string const renamePrefix{"__testcoca_rename_prefix__"};
};

}  // namespace ir_instrumenter
