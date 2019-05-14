#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
namespace {
    struct FuncBlockCount : public FunctionPass {
        static char ID;
        FuncBlockCount() : FunctionPass(ID){}

        bool runOnFunction (Function &F) override {
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

            errs()<<"Function "<<F.getName().str()+"\n";
            for (Loop *const L : LI)
                countBlocksInLoop(L, 0);
            return false;
        }
        void countBlocksInLoop (Loop *const L, unsigned nest) {
            unsigned num_Blocks = 0;
            Loop::block_iterator bb;
            for (bb = L->block_begin(); bb != L->block_end(); ++bb) {
                num_Blocks++;
            }
            errs()<<"Loop level "<<nest<<" has "<<num_Blocks<<" blocks\n";

            std::vector<Loop*> subLoops = L->getSubLoops();
            Loop::iterator j, f;
            for (j = subLoops.begin(), f = subLoops.end(); j !=f; ++j)
                countBlocksInLoop(*j, nest+1);
        }
        virtual void getAnalysisUsage(AnalysisUsage& AU) const override {
            AU.addRequired<LoopInfoWrapperPass>();
        }
    };
}//end of namespace
char FuncBlockCount::ID = 0;
static RegisterPass<FuncBlockCount> X("funcblockcount", "Function Count", false, false);
