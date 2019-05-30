#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"

using namespace llvm;
#define DEBUG_TYPE "myadce"
namespace {
    struct MYADCE : public FunctionPass {
        static char ID;
        MYADCE() : FunctionPass(ID){
            initializeMYADCEPass(*PassRegistry::getPassRegistry());
        }

        bool runOnFunction (Function &F) override {
            //This is the case when Attribute::OptimizeNone is set 
            //or when optimization bisect is over the limit.
            if (skipFunction(F))
                return false;

            std::set<Instruction*> Alive;
            std::vector<Instruction*> Worklist;
            Function *Func = &F;

            for (inst_iterator i = inst_begin(Func), e = inst_end(Func); i != e; ++i) {
                Instruction *I = &*i;
                //A TerminatorInst is representative of an instruction which terminates a basic block
                if (isa<TerminatorInst>(I) || isa<DbgInfoIntrinsic>(I)
                    || isa<LandingPadInst>(I) || I->mayHaveSideEffects()) {
                    Alive.insert(I);
                    Worklist.push_back(I);
                }
            }

            while (!Worklist.empty()) {
                Instruction *Curr = Worklist.back();
                Worklist.pop_back();
                for (Use &OI : Curr->operands()) {
                    if (Instruction *Inst = dyn_cast<Instruction>(OI))
                        if (Alive.insert(Inst).second)
                            Worklist.push_back(Inst);
                }
            }

            for (inst_iterator i = inst_begin(Func), e = inst_end(Func); i != e; ++i) {
                Instruction *I = &*i;
                if (!Alive.count(I)) {
                    Worklist.push_back(I);
                    I->dropAllReferences();
                }
            }

            for (Instruction *&I : Worklist) {
                I->eraseFromParent();
            }
            return !Worklist.empty();
        }
        virtual void getAnalysisUsage(AnalysisUsage& AU) const override {
            AU.setPreservesCFG();
        }
    };
}//end of namespace
//INITIALIZE_PASS_BEGIN(MyADCE, " MyADCE ", "myadce", false, false)
//INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
//INITIALIZE_PASS_END(MyADCE, "MyADCE", "myadce", false, false)
Pass *llvm::createMyADCEPass() { 
    return new MYADCE(); 
}
char MYADCE::ID = 0;
INITIALIZE_PASS(MYADCE, "myadce", "My Aggressive Dead Code Elimination", false, false)
