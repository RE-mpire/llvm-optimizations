#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/ConstantFold.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace {
    struct ConstantFolding  : public FunctionPass {
        static char ID;
        ConstantFolding() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
            errs() << "Constant Folding Working on function: " << F.getName() << "!\n";

            bool changed = false;
            std::vector<Instruction*> toDelete;

            for (auto &basicBlock : F) {
                for (auto &inst : basicBlock) {
                    if (isa<BinaryOperator>(&inst) &&
                        isa<ConstantInt>(inst.getOperand(0)) && 
                        isa<ConstantInt>(inst.getOperand(1))) {

                        auto *ins = cast<BinaryOperator>(&inst);
                        auto *op1 = cast<ConstantInt>(ins->getOperand(0));
                        auto *op2 = cast<ConstantInt>(ins->getOperand(1));

                        if (auto *res = ConstantFoldBinaryInstruction(ins->getOpcode(), op1, op2)) {
                                inst.replaceAllUsesWith(res);
                                toDelete.emplace_back(&inst);
                                changed = true;
                            }
                        }
                    else if (isa<ICmpInst>(&inst) &&
                             isa<ConstantInt>(inst.getOperand(0)) && 
                             isa<ConstantInt>(inst.getOperand(1))) {

                        auto *ins = cast<ICmpInst>(&inst);
                        auto *op1 = cast<ConstantInt>(ins->getOperand(0));
                        auto *op2 = cast<ConstantInt>(ins->getOperand(1));

                        if (auto *res = ConstantFoldCompareInstruction(ins->getPredicate(), op1, op2)) {
                            inst.replaceAllUsesWith(res);
                            toDelete.emplace_back(&inst);
                            changed = true;
                        }
                    }
                }
            }

            for (auto *inst : toDelete) inst->eraseFromParent();
            return changed;
        }
    };
}

char ConstantFolding::ID = 3;

static RegisterPass<ConstantFolding> X("const-fold", "Constant-Folding pass for minic", false, false);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerConstantFoldingPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
    PM.add(new ConstantFolding());
}
static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                       registerConstantFoldingPass);