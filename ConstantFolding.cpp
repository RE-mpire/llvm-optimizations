#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct ConstantFolding  : public FunctionPass {
        static char ID;
        ConstantFolding() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {

    };
}

char ConstantFolding::ID = 3;

static RegisterPass<ConstantFolding> X("const-fold", "Constant-Folding pass for minic", false, false);

static void registerConstantFoldingPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
    PM.add(new ConstantFolding());
}
static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                       registerConstantFoldingPass);