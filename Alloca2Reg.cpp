#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/CFG.h"

#include <unordered_set>
#include <unordered_map>
#include "llvm/IR/Constants.h"

using namespace llvm;

namespace {
    struct Alloca2RegPass : public FunctionPass {
        static char ID;
        Alloca2RegPass() : FunctionPass(ID) {}

        std::unordered_map<BasicBlock*, std::unordered_map<AllocaInst*, Value*>> Post;
        std::unordered_map<BasicBlock*, std::unordered_map<AllocaInst*, PHINode*>> Pre;
        std::unordered_set<AllocaInst*> TargetAllocas;
        std::vector<Instruction*>       ToDelete;

        inline PHINode *createPhi(BasicBlock *basicBlock, AllocaInst *alloca) {
            Type *type = alloca->getAllocatedType();
            PHINode *phi = PHINode::Create(type, 0, "phi", basicBlock->getFirstNonPHI());
            for (BasicBlock *predBB : predecessors(basicBlock)) {
                phi->addIncoming(UndefValue::get(type), predBB);
            }
            return phi;
        }

        inline bool isTargetAlloca(AllocaInst *alloca) {
            return TargetAllocas.find(alloca) != TargetAllocas.end();
        }

        bool optimizePhi(PHINode *phi) {
            const int numIncoming = phi->getNumIncomingValues();

            if (numIncoming == 0) {
                return true;
            }

            if (numIncoming == 1) {
                phi->replaceAllUsesWith(phi->getIncomingValue(0));
                return true;
            }

            // Check for self-references or undef values
            for (int i = 0; i < numIncoming; i++) {
                Value *value = phi->getIncomingValue(i);
                if (value == phi || dyn_cast<UndefValue>(value)) {
                    if (numIncoming <= 2) {
                        phi->removeIncomingValue(i, true);
                        phi->replaceAllUsesWith(phi->getIncomingValue(0));
                        return true;
                    }
                }
            }

            // Check if all incoming values are the same
            if (phi->hasConstantOrUndefValue()) {
                Value *firstValue = phi->getIncomingValue(0);
                if (dyn_cast<UndefValue>(firstValue)) {
                    phi->eraseFromParent();
                    return true;
                } else {
                    phi->replaceAllUsesWith(firstValue);
                    return true;
                }
            }

            return false;
        }

        void collectTargetAllocas(Function &F) {
            for (BasicBlock &basicBlock : F) {
                for (Instruction &inst : basicBlock) {
                    if (auto *allocaInst = dyn_cast<AllocaInst>(&inst)) {
                        if (allocaInst->getAllocatedType()->isIntegerTy()) {
                            TargetAllocas.emplace(allocaInst);
                            ToDelete.emplace_back(allocaInst);
                        }
                    }
                }
            }
        }

        virtual bool runOnFunction(Function &F) {
            errs() << "Alloca2Reg Working on function: " << F.getName() << "!\n";
            collectTargetAllocas(F);
            std::unordered_set<PHINode*> PhiNodes;

            if (TargetAllocas.empty()) {
                return false;
            }

            for (BasicBlock &basicBlock : F) {
                if (&basicBlock != &F.getEntryBlock()) {
                    for (auto &alloca : TargetAllocas) {
                        PHINode *phi = createPhi(&basicBlock, alloca);
                        Pre[&basicBlock][alloca] = phi;
                        Post[&basicBlock][alloca] = phi;
                        PhiNodes.emplace(phi);
                    }
                }
                
                for (Instruction &instruction : basicBlock) {
                    if (auto* loadInst = dyn_cast<LoadInst>(&instruction)) {
                        if (auto* alloca = dyn_cast<AllocaInst>(loadInst->getPointerOperand())) {
                            if (isTargetAlloca(alloca)) {
                                Value* value = Post[&basicBlock][alloca];
                                loadInst->replaceAllUsesWith(value);
                                ToDelete.emplace_back(&instruction);
                            }
                        }
                    } else if (auto* storeInst = dyn_cast<StoreInst>(&instruction)) {
                        if (auto* alloca = dyn_cast<AllocaInst>(storeInst->getPointerOperand())) {
                            if (isTargetAlloca(alloca)) {
                                Post[&basicBlock][alloca] = storeInst->getValueOperand();
                                ToDelete.emplace_back(&instruction);
                            }
                        }
                    }
                }
            }

            for (auto &map : Pre) {
                for (auto &[alloca, phi] : map.second) {
                    for (int i = 0; i < phi->getNumIncomingValues(); i++) {
                        if (Value *value = Post[phi->getIncomingBlock(i)][alloca]) {
                            phi->setIncomingValue(i, value);
                        }
                    }
                }
            }

            for (Instruction *inst : ToDelete) inst->eraseFromParent();
            ToDelete.clear();

            bool hasDeletedPhi;
            do {
                hasDeletedPhi = false;
                for (auto it = PhiNodes.begin(); it != PhiNodes.end();) {
                    PHINode *phi = *it;
                    if (phi->use_empty() && optimizePhi(phi)) {
                        it = PhiNodes.erase(it);
                        phi->eraseFromParent();
                        hasDeletedPhi = true;
                    } else {
                        ++it;
                    }
                }
            } while (hasDeletedPhi);
            
            for (auto &phi : PhiNodes) {
                if (phi->use_empty()) {
                    phi->eraseFromParent();
                }
            }

            Post.clear();
            Pre.clear();
            TargetAllocas.clear();
            return true;
        }
    };
}

char Alloca2RegPass::ID = 0;

static RegisterPass<Alloca2RegPass> X("alloca2reg", "Alloca-to-register pass for minic", false, false);

static void registerAlloca2RegPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
    PM.add(new Alloca2RegPass());
}
static RegisterStandardPasses
        RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                       registerAlloca2RegPass);