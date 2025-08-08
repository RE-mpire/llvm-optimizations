#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/CFG.h"

using namespace llvm;

namespace {
    struct AlgebraicSimplificationPass  : public FunctionPass {
        static char ID;

    };
}

char AlgebraicSimplificationPass::ID = 1;

static RegisterPass<AlgebraicSimplificationPass> X("alg-simplify", "Algebraic-Simplification pass for minic", false, false);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerAlgebraicSimplificationPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
    PM.add(new AlgebraicSimplificationPass());
}
static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                       registerAlgebraicSimplificationPass);