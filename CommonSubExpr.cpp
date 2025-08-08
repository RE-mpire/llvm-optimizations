#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <unordered_set>

using namespace llvm;

namespace {
    struct CommonSubExprPass : public FunctionPass {
        static char ID;
        CommonSubExprPass() : FunctionPass(ID) {}

        std::unordered_set<Instruction*> ToDelete;
        
        virtual bool runOnFunction(Function &F) {
            errs() << "Running CommonSubExpr on function: " << F.getName() << "!\n";
            do {
                ToDelete.clear();
                for (auto &basicBlock : F) {
                    for (auto it1 = basicBlock.begin(); it1 != basicBlock.end(); ++it1) {
                        Instruction *I1 = &(*it1);
                        if (isa<AllocaInst>(I1)) continue;

                        llvm::Value *loadOp = isa<LoadInst>(I1) ? cast<LoadInst>(I1)->getPointerOperand() : nullptr;

                        for (auto it2 = std::next(it1); it2 != basicBlock.end(); ++it2) {
                            Instruction *I2 = &(*it2);
                            auto *storeInst = dyn_cast<StoreInst>(I2);
                            if (storeInst && storeInst->getPointerOperand() == loadOp) break;

                            if (I1 != I2 && !ToDelete.count(I1) && !ToDelete.count(I2) && I1->isIdenticalTo(I2)) {
                                ToDelete.emplace(I2);
                                I2->replaceAllUsesWith(I1);
                            }
                        }
                    }
                }
                for (Instruction *inst : ToDelete) inst->eraseFromParent();
            } while (!ToDelete.empty());
            return true;

        }
    };
}

char CommonSubExprPass::ID = 2;

static RegisterPass<CommonSubExprPass> X("CSE", "Common Sub-expression Elimination pass for minic", false, false);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerCommonSubExprPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
     PM.add(new CommonSubExprPass()); 
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerCommonSubExprPass);