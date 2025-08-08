#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct LoopUnrollPass : public LoopPass {
        static char ID;
        LoopUnrollPass() : LoopPass(ID) {}

        bool runOnLoop(Loop *L, LPPassManager &LPM) override {
            if (!L || !L->getLoopPreheader()) {
                return false;
            }

            auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
            auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
            auto &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*L->getHeader()->getParent());
            auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(*L->getHeader()->getParent());
        
            UnrollLoopOptions ULO;
            ULO.Runtime = false;
            ULO.Force = true;
            ULO.Count = SE.getSmallConstantTripCount(L);
            if (ULO.Count == 0) {
                ULO.Count = 8; // Fallback if unknown
            }

            Loop *RemainderLoop = nullptr;
            bool Changed = UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, nullptr, /*PreserveLCSSA=*/true, &RemainderLoop) != LoopUnrollResult::Unmodified;
            errs() << "Unrolling loop: " << (Changed ? "Success" : "Failed") << "\n";
            if (Changed) {
                simplifyLoopAfterUnroll(L, /*SimplifyIVs=*/true, &LI, &SE, &DT, &AC, &TTI);
            }

            return Changed;
        }

        void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.addRequired<LoopInfoWrapperPass>();
            AU.addRequired<ScalarEvolutionWrapperPass>();
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addRequired<AssumptionCacheTracker>();
            AU.addRequired<TargetTransformInfoWrapperPass>();
            AU.addPreserved<LoopInfoWrapperPass>();
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
