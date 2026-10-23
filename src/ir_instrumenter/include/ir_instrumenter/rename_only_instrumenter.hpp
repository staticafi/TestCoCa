#pragma once

#include <ir_instrumenter/llvm_instrumenter.hpp>

namespace ir_instrumenter {

// Performs no instrumentation of its own. Used by pipelines that already
// instrumented the source (e.g. decision coverage) and only need the shared
// renaming step so the target runtime can take over main().
class rename_only_instrumenter : public llvm_instrumenter {
public:
    void instrument(llvm::Module& /*M*/) override {}
};

}  // namespace ir_instrumenter
