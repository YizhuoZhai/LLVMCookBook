/*--------------------------------
*   * The lexer, AST and parser
*    --------------------------- */
#include <cstdlib>
#include <string>
#include <string.h>
#include <vector>
#include <cctype>
#include <iostream>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
using namespace std;
//Use enumeration for token types
enum Token_Type {
        EOF_TOKEN = 0,
        NUMERIC_TOKEN,    //current token is of numeric type
        IDENTIFIER_TOKEN, //identifier
        PARAN_TOKEN,      //parenthesis
        DEF_TOKEN         //funciton definition
};
//Hold the numeric value
static int Numeric_Val;
//The name of Identifier
static std::string Identifier_string;
FILE *file;
//All function and variables of the code
static llvm::Module *Module_Ob;
static llvm::LLVMContext TheGlobalContext;
//Generate the LLVM IR and record the current program point
static llvm::IRBuilder<> Builder(TheGlobalContext);
//Symbol table, record the defined value in current scope
//as well as the function arguments.
static std::map<std::string, llvm::Value*> Named_Values;
static llvm::FunctionPassManager *Global_FP;

//syntax analysis function
static int get_token()
{
	//define the first char
        static int LastChar = ' ';
	//skip the space
        while (isspace(LastChar))
                LastChar = fgetc(file);
	//identify the identifier token
        if (isalpha(LastChar))
        {
                Identifier_string = LastChar;

                while (isalnum(LastChar = fgetc(file)))
                        Identifier_string += LastChar;

                if (Identifier_string == "def") {
                	std::cout<<"string: "<<Identifier_string<<": "<<"DEF_TOKEN"<<"\n"; 
		       	return DEF_TOKEN;
		}
                std::cout<<"string: "<<Identifier_string<<": "<<"IDENTIFIER_TOKEN"<<"\n";
                return IDENTIFIER_TOKEN;
        }

	//identify the numeric token
        if (isdigit(LastChar))
        {
                std::string NumStr;
                do
                {
                        NumStr += LastChar;
                        LastChar = fgetc(file);
                } while (isdigit(LastChar));

                //turn string to int
                Numeric_Val = strtod(NumStr.c_str(), 0);
                std::cout<<"Numeric_Val: "<<Numeric_Val<<": "<<"NUMERIC_TOKEN"<<"\n";
                return NUMERIC_TOKEN;
        }
        //identify the comment, skip it and analyze it
        if(LastChar == '#')
        {
                do{
                        LastChar = fgetc(file);
                }while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');
		//EOF is generally -1 in C++
                if(LastChar != EOF)
                        return get_token();
        }
        if(LastChar == EOF) {
		std::cout<<"EOF_TOKEN\n";
                return EOF_TOKEN;
	}
        //Other characters, such as comma, parenthesis
        int ThisChar = LastChar;
        LastChar = fgetc(file);
	//char realChar = LastChar;
	std::cout<<"Other Case: "<<ThisChar<<": "<<"OTHERS"<<"\n";
        return ThisChar;
}
//AST for different types
class BaseAST
{
        public:
        virtual ~BaseAST();
	//This value is a SSA object
        virtual llvm::Value* Codegen() = 0;
};
BaseAST::~BaseAST(){}
//AST class for variable expressions, name matters most
class VariableAST : public BaseAST
{
        std::string Var_Name;
        public:
                VariableAST(std::string &name) : Var_Name(name){}
                virtual llvm::Value* Codegen();
};
//AST for numeric expressions: value matters
class NumericAST : public BaseAST
{
        int numeric_val;
        public:
                NumericAST(int val) : numeric_val(val) {}
        virtual llvm::Value *Codegen();
};

llvm::Value *NumericAST::Codegen()
{
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheGlobalContext), numeric_val);
}
llvm::Value *VariableAST::Codegen()
{
        llvm::Value *V = Named_Values[Var_Name];
        return V? V : 0;
}
//binary operation expressions: op and two operands matter
class BinaryAST : public BaseAST
{
        std::string Bin_Operator;
        BaseAST *LHS, *RHS;
        public:
        BinaryAST(std::string op, BaseAST *lhs, BaseAST *rhs): 
                Bin_Operator(op), LHS(lhs), RHS(rhs) {}
        virtual llvm::Value* Codegen();
};

llvm::Value *BinaryAST::Codegen()
{
        llvm::Value *L = LHS->Codegen();
        llvm::Value *R = RHS->Codegen();
        if(L == 0 || R == 0) return 0;

        switch(atoi(Bin_Operator.c_str()))
        {
                case '+': return Builder.CreateAdd(L, R, "addtmp");
                case '-': return Builder.CreateSub(L, R, "subtmp");
                case '*': return Builder.CreateMul(L, R, "multmp");
                case '/': return Builder.CreateUDiv(L, R, "divtmp");
                default: return 0;
        }
}
//Function declaration: name and arg vector, why no ret value.
class FunctionDeclAST
{
        std::string Func_Name;
        std::vector<std::string> Arguments;
        public:
                FunctionDeclAST(const std::string &name, const std::vector<std::string> &args):
                        Func_Name(name), Arguments(args) {
			cout<<"Initialize a FunctionDeclAST:\n";
			cout<<"Func_Name: "<<Func_Name<<"\n";
			for (auto arg : Arguments)
				cout<<"arg: "<<arg<<"\n";
	}
        virtual llvm::Function* Codegen();
};
//Function definition: declaration (name and arg vector) and body.
class FunctionDefnAST
{
        FunctionDeclAST *Func_Decl;
        BaseAST* Body;
        public:
                FunctionDefnAST(FunctionDeclAST *proto, BaseAST *body) : Func_Decl(proto), Body(body){}
        virtual llvm::Function* Codegen();
};
//Function call: name of callee and function arguments
class FunctionCallAST : public BaseAST
{
        std::string Function_Callee;
        std::vector<BaseAST *> Function_Arguments;
        public:
                FunctionCallAST(const std::string &Callee, std::vector<BaseAST*> &args):
                        Function_Callee(Callee), Function_Arguments(args){}
        virtual llvm::Value* Codegen();
};
llvm::Value *FunctionCallAST::Codegen()
{
        llvm::Function *CalleeF = Module_Ob->getFunction(Function_Callee);
        vector<llvm::Value*> ArgsV;
        for(unsigned i =0, e = Function_Arguments.size(); i != e; i++)
        {
                ArgsV.push_back(Function_Arguments[i] -> Codegen());
                if(ArgsV.back() == 0) return 0;
        }
        return Builder.CreateCall(CalleeF, ArgsV, "calltemp");
}
llvm::Function *FunctionDeclAST::Codegen()
{
	std::cout<<"Inside FunctionDeclAST \n";
        vector<llvm::Type*> Integers(Arguments.size(), llvm::Type::getInt32Ty(TheGlobalContext));
        llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheGlobalContext), Integers, false);
        llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Func_Name, Module_Ob);
	std::cout<<"F->getName() : "<<F->getName().str()<<", Func_Name: "<<Func_Name<<"\n";
        if(F->getName().str() != Func_Name)
        {
		std::cout<<"erase from parent.\n";
                F -> eraseFromParent();
                F = Module_Ob->getFunction(Func_Name);

                if(!F->empty())
                        return 0;
                if(F -> arg_size() != Arguments.size())
                        return 0;
        }
        unsigned Idx = 0;
        for(llvm::Function::arg_iterator Arg_It = F->arg_begin(); Idx != Arguments.size(); ++Arg_It, ++Idx)
        {
                Arg_It->setName(Arguments[Idx]);
                Named_Values[Arguments[Idx]] = Arg_It;
        }
        return F;
}
llvm::Function *FunctionDefnAST::Codegen()
{
	cout<<"Inside Function Definition AST Codegen : \n";
        Named_Values.clear();

        llvm::Function *TheFunction = Func_Decl->Codegen();
	llvm::errs()<<"The Function: \n"<<*TheFunction<<"\n";
        if(TheFunction == 0) {
		cout<<"TheFunction == 0.\n";
                return 0;
	}
        llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheGlobalContext, "entry", TheFunction);
        Builder.SetInsertPoint(BB);

        if(llvm::Value *RetVal = Body->Codegen())
        {
                Builder.CreateRet(RetVal);
                verifyFunction(*TheFunction);
                //Global_FP->run(*TheFunction);
                return TheFunction;
        }

        TheFunction->eraseFromParent();
        return 0;
}
/*-----------
* Parser
-----------*/
//store in the Current token 
static int Current_token;
//get the next token from lexer
static int next_token()
{
        Current_token = get_token();
        return Current_token;
}

static BaseAST* paran_parser();
static BaseAST* Base_Parser();
static BaseAST* binary_op_parser(int, BaseAST *);

static BaseAST *numeric_parser()
{
        BaseAST *Result = new NumericAST(Numeric_Val);
        next_token();
        return Result;
}
static BaseAST* expression_parser()
{
	cout<<"Inside expression_parser()\n";
        BaseAST *LHS = Base_Parser();
        if(!LHS) {
		cout<<"!LHS\n";
                return 0;
	}
        return binary_op_parser(0, LHS);
}
static BaseAST* identifier_parser()
{
        std::string IdName = Identifier_string;
        next_token();
        if(Current_token != '(')
                return new VariableAST(IdName);

        next_token();

        std::vector<BaseAST*> Args;
        if(Current_token != '(')
        {
                while(1)
                {
                        BaseAST *Arg = expression_parser();
                        if(!Arg) return 0;
                        Args.push_back(Arg);

                        if(Current_token == ')') break;

                        if(Current_token != ',')
                                return 0;
                        
                        next_token();
                }
        }
        next_token();

        return new FunctionCallAST(IdName, Args);
}
//return different parser for current token
static BaseAST* Base_Parser()
{
	cout<<"Inside Base_Parser()\n";
	cout<<"Current_token:"<<Current_token<<"\n";
        switch(Current_token)
        {
                case IDENTIFIER_TOKEN : return identifier_parser();
                case NUMERIC_TOKEN : return numeric_parser();
                case '(' : return paran_parser();
		default: return 0;
        }
}

static FunctionDeclAST *func_decl_parser()
{
	cout<<"Inside func_decl_parser():\n";
        if(Current_token != IDENTIFIER_TOKEN)
                return 0;
        
        std::string FnName = Identifier_string;
	cout<<"FnName: "<<FnName<<"\n";
        next_token();

        if(Current_token != '(')
                return 0;

        std::vector<std::string> Function_Argument_Names;

        while(next_token() == IDENTIFIER_TOKEN)
        {
                Function_Argument_Names.push_back(Identifier_string);
		cout<<"Identifier_string: "<<Identifier_string<<"\n";
        }

        if(Current_token != ')')
                return 0;
        next_token();
	cout<<"create FunctionDeclAST.\n";
        return new FunctionDeclAST(FnName, Function_Argument_Names);
}
static FunctionDefnAST *func_defn_parser()
{
	cout<<"Inside function func_defn_parser():\n";
        next_token();
        FunctionDeclAST *Decl = func_decl_parser();
        if(Decl == 0)
                return 0;
        
        if(BaseAST *Body = expression_parser())
        {
		cout<<"new FunctionDefnAST: \n";
                return new FunctionDefnAST(Decl, Body);
        }
        return 0;
}
//Set the priority of the binary operator:
static std::map<char, int> Operator_Precedence;
static void init_precedence()
{
        Operator_Precedence['-'] = 1;
        Operator_Precedence['+'] = 2;
        Operator_Precedence['/'] = 3;
        Operator_Precedence['*'] = 4;
}
static int getBinOpPrecedence()
{
        if (!isascii(Current_token))
                return -1;
        int TokPrec = Operator_Precedence[Current_token];
        if(TokPrec <= 0) return -1;

        return TokPrec;
}
static BaseAST* binary_op_parser(int Old_Prec, BaseAST *LHS)
{
	cout<<"Inside binary_op_parser():\n";
        while(1)
        {
		// the binary op is not the prefix expression, why it get the op
		//directly at the begining.
                int Operator_Prec = getBinOpPrecedence();
		cout<<"Operator_Prec :"<<Operator_Prec<<"\n";
                if(Operator_Prec < Old_Prec)
                        return LHS;
                int BinOp = Current_token;
		cout<<"BinOp : "<<BinOp<<"\n";
                next_token();

                BaseAST* RHS = Base_Parser();
                if(!RHS) 
                        return 0;

                int Next_Prec = getBinOpPrecedence();
		cout<<"Next_Prec :"<<Next_Prec<<"\n";
                if(Operator_Prec < Next_Prec)
                {
                        RHS = binary_op_parser(Operator_Prec + 1, RHS);
                        if(RHS == 0)
                                return 0;
                }
                LHS = new BinaryAST(std::to_string(BinOp), LHS, RHS);
        }
	cout<<"End of binary_op_parser.\n";
}
static BaseAST* paran_parser()
{
        next_token();
        BaseAST* V = expression_parser();
        if(!V)
                return 0;
        
        if(Current_token != ')')
                return 0;
        return V;
}
static void HandleDefn()
{
        if(FunctionDefnAST *F = func_defn_parser())
        {
                if(llvm::Function *LF = F -> Codegen())
                {
			
                }
        }
        else
        {
                next_token();
        }
}
static FunctionDefnAST *top_level_parser()
{
	cout<<"Inside top_level_parser()\n";
        if (BaseAST *E = expression_parser()) {
                FunctionDeclAST *Func_Decl =
                        new FunctionDeclAST("", std::vector<std::string>());
                return new FunctionDefnAST(Func_Decl, E);
        }
        return 0;

}
static void HandleTopExpression()
{
	cout<<"Inside HandleTopExpression():\n";
        if(FunctionDefnAST *F = top_level_parser())
        {
                if(llvm::Function *LF = F ->Codegen())
                {

                }
        }
        else
        {
                next_token();
        }
}
//Driver function
static void Driver()
{
	cout<<"Inside Driver :\n";
        while(1)
        {
                //printf("Current_token: %c \n", Current_token);
		cout<<"Current_token : "<<Current_token<<"\n";
                switch(Current_token)
                {       
                        case EOF_TOKEN: return;
                        case ';': next_token(); break;
			//Handle definition
                        case DEF_TOKEN: {
				std::cout<<"HandleDefn: \n";
				HandleDefn(); 
				break;
			}
			//Handle expression
                        default: {
				std::cout<<"handleTopExpr: \n";
				HandleTopExpression(); 
				break;
			}
                }
        }
}

int main(int argc, char *argv[])
{
        llvm::LLVMContext &Context = TheGlobalContext;
        init_precedence();
        file = fopen(argv[1], "r");
        if(file == 0)
        {
             printf("File not found.\n");
        }
        next_token();
        Module_Ob = new llvm::Module(argv[1], Context);
        //llvm::FunctionPassManager FP(Module_Ob);
        //Global_FP = &FP;
        Driver();
        Module_Ob->dump();
        //Module_Ob->print(llvm::outs(), nullptr);
        return 0;
}

