#include "llvm/Transforms/IPO.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/InlineCost.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

namespace{
  class MyInliner : public LegacyInlinerBase {
    //InlineCostAnalysis *ICA;
    public:
      MyInliner() : LegacyInlinerBase(ID){
        initializeMyInlinerPass(*PassRegistry::getPassRegistry());
      }

      static char ID;
      InlineCost getInlineCost(CallSite CS);
      //void getAnalysisUsage(AnalysisUsage &AU) const override;
      bool runOnSCC(CallGraphSCC &SCC);
      using llvm::Pass::doFinalization;

      bool doFinalization(CallGraph &CG) {
        return removeDeadFunctions(CG, /*AlwaysInlineOnly=*/
                true);
      }
  };
}
char MyInliner::ID = 0;
INITIALIZE_PASS_BEGIN(MyInliner, "my-inline",
                      "Inliner for always_inline functions", false, false)
//INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(InlineCostAnalysis)
INITIALIZE_PASS_END(MyInliner, "my-inline",
                    "Inliner for always_inline functions", false, false)
Pass *llvm::createMyInlinerPass() { return new MyInliner(); }
/*Pass *llvm::createMynlinerPass(bool InsertLifetime) {
  return new MyInliner(InsertLifetime);
}*/
InlineCost MyInliner::getInlineCost(CallSite CS){
  Function *Callee = CS.getCalledFunction();
  if (Callee && !Callee->isDeclaration() &&
      CS.hasFnAttr(Attribute::AlwaysInline) &&
      isInlineViable(*Callee))
        return InlineCost::getAlways();
  return InlineCost::getNever();
}
bool MyInliner::runOnSCC(CallGraphSCC &SCC){
  //ICA = &llvm::AAResultsWrapperPass::getAnalysis<InlineCostAnalysis>();
  return inlineCalls(SCC);
  //return Inliner::runOnSCC(SCC);
}/*
void MyInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<InlineCostAnalysis>();
  Inliner::getAnalysisUsage(AU);
}*/