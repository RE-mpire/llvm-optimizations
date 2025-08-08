#include "llvm/IR/CFG.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct CommonSubExprPass : public FunctionPass {
        static char ID;
        CommonSubExprPass() : FunctionPass(ID) {}

    };
}

char CommonSubExprPass::ID = 2;

static RegisterPass<CommonSubExprPass> X("CSE", "Common Sub-expression Elimination pass for minic", false, false);

static void registerCommonSubExprPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
     PM.add(new CommonSubExprPass()); 
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerCommonSubExprPass);