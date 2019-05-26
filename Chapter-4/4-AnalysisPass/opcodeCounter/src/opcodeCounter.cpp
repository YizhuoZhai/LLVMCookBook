#define DEBUG_TYPE "opcodeCounter"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
using namespace llvm;

namespace{
    struct CountOpcode: public FunctionPass {
        std::map< std::string, int> opcodeCounter;
        static char ID;
        CountOpcode () : FunctionPass(ID) {}
        virtual bool runOnFunction (Function &F) {
            llvm::outs() << "Function " << F.getName () << '\n';
            for ( Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                for ( BasicBlock::iterator i = bb->begin(), e = bb->end(); i!= e; ++i) {
                    if(opcodeCounter.find(i->getOpcodeName()) == opcodeCounter.end()) {
                        opcodeCounter[i->getOpcodeName()] = 1;
                    } else {
                        opcodeCounter[i->getOpcodeName()] += 1;
                    }
                }
            }
            std::map< std::string, int>::iterator i = opcodeCounter.begin();
            std::map< std::string, int>::iterator e = opcodeCounter.end();
            while (i != e) {
                llvm::outs() << i->first << ": " << i->second << "\n";
                i++;
            }
            llvm::outs() << "\n";
            opcodeCounter.clear();
            return false;
        }
    };
}
char CountOpcode::ID = 0;
static RegisterPass<CountOpcode> X("opcodeCounter", "Count number of opcode in a functions");