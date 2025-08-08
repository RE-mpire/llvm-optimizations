#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"

using namespace llvm;

namespace {
    struct AlgebraicSimplificationPass  : public FunctionPass {
        static char ID;
        AlgebraicSimplificationPass() : FunctionPass(ID) {}

        Value *eval(Value *value) {
            if (auto *binop = dyn_cast<BinaryOperator>(value)) {
            Value *L = binop->getOperand(0);
            Value *R = binop->getOperand(1);

            // Simplify binary operations with constant operands
            if (auto *LConst = dyn_cast<ConstantInt>(L)) {
                if (auto *RConst = dyn_cast<ConstantInt>(R)) {
                APInt LVal = LConst->getValue();
                APInt RVal = RConst->getValue();

                switch (binop->getOpcode()) {
                    case Instruction::Add:
                    if (LVal == 0) return R;
                    if (RVal == 0) return L;
                    case Instruction::Sub:
                    if (RVal == 0) return L;
                    case Instruction::Mul:
                    if (LVal == 1) return R;
                    if (RVal == 1) return L;
                    if (LVal == 0 || RVal == 0) return ConstantInt::get(L->getType(), 0);
                    case Instruction::SDiv:
                    case Instruction::UDiv:
                    if (RVal == 1) return L;
                }
                }
            }

            // Simplify logical AND/OR operations
            if (auto *LConst = dyn_cast<ConstantInt>(L)) {
                if (binop->getOpcode() == Instruction::And) {
                if (LConst->isZero()) return ConstantInt::getFalse(L->getType());
                if (LConst->isOne()) return R;
                } else if (binop->getOpcode() == Instruction::Or) {
                if (LConst->isZero()) return R;
                if (LConst->isOne()) return ConstantInt::getTrue(L->getType());
                }
            }
            if (auto *RConst = dyn_cast<ConstantInt>(R)) {
                if (binop->getOpcode() == Instruction::And) {
                if (RConst->isZero()) return ConstantInt::getFalse(L->getType());
                if (RConst->isOne()) return L;
                } else if (binop->getOpcode() == Instruction::Or) {
                if (RConst->isZero()) return L;
                if (RConst->isOne()) return ConstantInt::getTrue(L->getType());
                }
            }
            }
            return nullptr;
        }

        virtual bool runOnFunction(Function &F) {
            errs() << "Algebraic Simplification Working on function: " << F.getName() << "!\n";

            bool changed;
            do {
                changed = false;
                for (BasicBlock &basicBlock : F) {
                    for (Instruction &inst : basicBlock) {
                        if (Value *value = eval(&inst)) {
                            inst.replaceAllUsesWith(value);
                            inst.eraseFromParent();
                            changed = true;
                        }
                    }
                }
            } while (changed);
            return changed;
            
        }
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