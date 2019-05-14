#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
namespace {
    struct FuncBlockCount : public FunctionPass {
        static char ID;
        FuncBlockCount() : FunctionPass(ID){}

        bool runOnFunction (Function &F) override {
            errs()<<"Function "<<F.getName()<<'\n';
            return false;
        }
    };
}//end of namespace
char FuncBlockCount::ID = 0;
static RegisterPass<FuncBlockCount> X("funcblockcount", "Function Count", false, false);
