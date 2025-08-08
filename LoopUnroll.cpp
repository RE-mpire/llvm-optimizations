#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct LoopUnrollPass : public LoopPass {
        static char ID;
        LoopUnrollPass() : LoopPass(ID) {}

        bool runOnLoop(Loop *L, LPPassManager &LPM) override {

        }
    };

    char LoopUnrollPass::ID = 1;

    static RegisterPass<LoopUnrollPass>
        X("simple-loop-unroll", "Loop Unrolling Pass", false, false);

    static void registerLoopUnrollPass(const PassManagerBuilder &,
                                       legacy::PassManagerBase &PM) {
        PM.add(new LoopUnrollPass());
    }

    static RegisterStandardPasses RegisterMyPass(
        PassManagerBuilder::EP_EarlyAsPossible, registerLoopUnrollPass);
}
