/*-----------------------------------------------------
*   * The lexer and AST and parser
*   * if-else handled by Clang:
*   *clang.llvm.org/doxygen/classclang_1_1IfStmt.html. 
*    ----------------------------------------------- */
#include <cstdlib>
#include <string>
#include <string.h>
#include <vector>
#include <cctype>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
using namespace std;
enum Token_Type {
        EOF_TOKEN = 0,
        NUMERIC_TOKEN,    //current token is of numeric type
        IDENTIFIER_TOKEN, //identifier
        PARAN_TOKEN,      //parenthesis
        DEF_TOKEN,         //funciton definition
        IF_TOKEN,
        THEN_TOKEN,
        ELSE_TOKEN,
	FOR_TOKEN,
	IN_TOKEN,
        BINARY_TOKEN,
        UNARY_TOKEN
};
static int Numeric_Val;
static std::string Identifier_string;
FILE *file;
static llvm::Module *Module_Ob;
static llvm::LLVMContext TheGlobalContext;
static llvm::IRBuilder<> Builder(TheGlobalContext);
static std::map<std::string, llvm::Value*> Named_Values;
static llvm::FunctionPassManager *Global_FP;
static std::map<char, int> Operator_Precedence;
static llvm::ExecutionEngine *TheExeEngine;
static void init_precedence()
{
        Operator_Precedence['<'] = 0;
        Operator_Precedence['-'] = 1;
        Operator_Precedence['+'] = 2;
        Operator_Precedence['/'] = 3;
        Operator_Precedence['*'] = 4;
}
static int get_token()
{
        //printff("Inside get_token:\n");
        static int LastChar = ' ';

        while (isspace(LastChar))
                LastChar = fgetc(file);
        if (isalpha(LastChar))
        {
                Identifier_string = LastChar;

                while (isalnum(LastChar = fgetc(file)))
                        Identifier_string += LastChar;

                //printff("Identifier_string = %s\n", Identifier_string.c_str());
                if (Identifier_string == "def") {
                        //printff("string: %s : DEF_TOKEN\n", Identifier_string.c_str());
                        return DEF_TOKEN;
                }
                if (Identifier_string == "if") {
                        //printff("string: %s : IF_TOKEN\n", Identifier_string.c_str());
                        return IF_TOKEN;
                }
                if(Identifier_string == "then") {
                        //printff("string: %s : THEN_TOKEN\n", Identifier_string.c_str());
                        return THEN_TOKEN;
                }
                if(Identifier_string == "else") {
                        //printff("string: %s : ELSE_TOKEN\n", Identifier_string.c_str());
                        return ELSE_TOKEN;
                }
		if(Identifier_string == "for") {
                        //printff("string: %s : FOR_TOKEN\n", Identifier_string.c_str());
                        return FOR_TOKEN;
                }
                        
                if(Identifier_string == "in") {
                        //printff("string: %s : IN_TOKEN\n", Identifier_string.c_str());
                        return IN_TOKEN;
                }
                if(Identifier_string == "binary") {
                        //printff("string: %s : BINARY_TOKEN\n", Identifier_string.c_str());
                        return BINARY_TOKEN;
                }
                if(Identifier_string == "unary") {
                        //printff("string: %s : UNARY_TOKEN\n", Identifier_string.c_str());
                        return UNARY_TOKEN;
                }
                //printff("string: %s : IDENTIFIER_TOKEN\n", Identifier_string.c_str());
                //std::cout<<"string: "<<Identifier_string<<": "<<"IDENTIFIER_TOKEN"<<"\n";
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
                //printff("Numeric_Val: %d : NUMERIC_TOKEN\n", Numeric_Val);
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
		//printff("EOF_TOKEN\n");
                return EOF_TOKEN;
	}
        //Other characters, such as comma, parenthesis
        int ThisChar = LastChar;
        LastChar = fgetc(file);
	//char realChar = LastChar;
        //printff("Other Case: %d : OTHERS\n", ThisChar);
        return ThisChar;
}
//AST for different types
class BaseAST
{
        public:
        virtual ~BaseAST();
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
//for numeric expressions: value matters
class NumericAST : public BaseAST
{
        int numeric_val;
        public:
                NumericAST(int val) : numeric_val(val) {}
        virtual llvm::Value *Codegen();
};
class ExprIfAST : public BaseAST{
        BaseAST *Cond, *Then, *Else;
        public:
            ExprIfAST(BaseAST *cond, BaseAST *then, BaseAST *else_st)
                : Cond(cond), Then(then), Else(else_st){}
            llvm::Value *Codegen() override;
};
class ExprForAST : public BaseAST{
	std::string Var_Name;
	BaseAST *Start, *End, *Step, *Body;
	public:
	    ExprForAST(const std::string &varname, BaseAST *start, BaseAST *end,
			BaseAST *step, BaseAST *body)
			:Var_Name(varname),Start(start),End(end),Step(step), Body(body){}
	    llvm::Value *Codegen() override;
};
class ExprUnaryAST : public BaseAST {
        char Opcode;
        BaseAST *Operand;
        //BookVersion: BaseAST->BaseAST*
        public: 
                ExprUnaryAST(char opcode, BaseAST* operand) : Opcode(opcode), Operand(operand){}
        virtual llvm::Value *Codegen();
};
llvm::Value *ExprIfAST :: Codegen()
{
        llvm::Value *Condtn = Cond->Codegen();
        if(Condtn == 0)
                return 0;
        Condtn = Builder.CreateICmpNE(Condtn, Builder.getInt32(0), "ifcond");
        llvm::Function *TheFunc = Builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheGlobalContext, "then", TheFunc);
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheGlobalContext, "else");
        llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheGlobalContext, "ifcont");

        Builder.CreateCondBr(Condtn, ThenBB, ElseBB);
        Builder.SetInsertPoint(ThenBB);

        llvm::Value *ThenVal = Then->Codegen();
        if(ThenVal == 0)
                return 0;
        
        Builder.CreateBr(MergeBB);
        ThenBB = Builder.GetInsertBlock();

        TheFunc->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
        
        llvm::Value *ElseVal = Else->Codegen();
        if(ElseVal == 0)
                return 0;
        Builder.CreateBr(MergeBB);
        ElseBB = Builder.GetInsertBlock();

        TheFunc->getBasicBlockList().push_back(MergeBB);
        Builder.SetInsertPoint(MergeBB);
        llvm::PHINode *Phi = Builder.CreatePHI(llvm::Type::getInt32Ty(TheGlobalContext), 2, "iftmp");

        Phi->addIncoming(ThenVal, ThenBB);
        Phi->addIncoming(ElseVal, ElseBB);
        return Phi;
}
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
                case '<': 
                        L = Builder.CreateICmpULT(L, R, "cmptmp");
                        return Builder.CreateZExt(L, llvm::Type::getInt32Ty(TheGlobalContext), "booltmp");
                default: break;
        }
        //BookError: "Op" Undefined
        llvm::Function *F = Module_Ob->getFunction(std::string("binary")+Bin_Operator.c_str());
        llvm::Value *Ops[2] = {L, R};
        return Builder.CreateCall(F, Ops, "binop");
}
llvm::Value *ExprForAST::Codegen(){
	llvm::Value *StartVal = Start->Codegen();
	if(StartVal == 0)
		return 0;
                
	llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
	llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
	llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheGlobalContext, "loop", TheFunction);

        Builder.CreateBr(LoopBB);
        Builder.SetInsertPoint(LoopBB);
        //BookError: PHINode -> PHINode*
        llvm::PHINode *Variable = Builder.CreatePHI(llvm::Type::getInt32Ty(TheGlobalContext), 2, Var_Name.c_str());
        Variable->addIncoming(StartVal, PreheaderBB);
        
        llvm::Value *OldVal = Named_Values[Var_Name];
        Named_Values[Var_Name] = Variable;

        if (Body->Codegen() == 0) {
                return 0;
        }

        llvm::Value *StepVal;
        if (Step) {
                StepVal = Step->Codegen();
                if (StepVal == 0)
                        return 0;
        }
        else {
                StepVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheGlobalContext), 1);
        }

        llvm::Value *NextVar = Builder.CreateAdd(Variable, StepVal, "nextvar");

        llvm::Value *EndCond = End->Codegen();
        if (EndCond == 0)
                return EndCond;
        EndCond = Builder.CreateICmpNE(EndCond, llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheGlobalContext), 0), "loopcond");

        llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
        llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheGlobalContext, "afterloop", TheFunction);

        Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
        Builder.SetInsertPoint(AfterBB);
        Variable->addIncoming(NextVar, LoopEndBB);
        if (OldVal) {
                Named_Values[Var_Name] = OldVal;
        }
        else {
                Named_Values.erase(Var_Name);
        }
        return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(TheGlobalContext));
}
llvm::Value * ExprUnaryAST::Codegen() {
        llvm::Value *OperandV = Operand->Codegen();
        if (OperandV == 0) return 0;
        llvm::Function *F = Module_Ob->getFunction(std::string("unary") + Opcode);

        if (F == 0)
                return 0;
        return Builder.CreateCall(F, OperandV, "unop");
}
//Function declaration: name and arg vector, why no ret value.
//Also the binary and unary operations
class FunctionDeclAST
{
        std::string Func_Name;
        std::vector<std::string> Arguments;
        bool isOperator;
        unsigned Precedence;

        public:
                FunctionDeclAST(const std::string &name, const std::vector<std::string> &args, 
                                bool isoperator = false, unsigned prec = 0):
                        Func_Name(name), Arguments(args), isOperator(isoperator), Precedence(prec) {
                                //printff("Initialize a FunctionDeclAST:\n");
                                //printff("Func_Name : %s\n", Func_Name.c_str());
                                //for (auto arg : Arguments)
                                //       printf("Arg : %s\n", arg.c_str()); 
                        }
                bool isUnaryOp() const {return isOperator && Arguments.size()==1;}
                bool isBinaryOp() const {return isOperator && Arguments.size()==2;}
                
                char getOperatorName() const {
                        assert(isUnaryOp() || isBinaryOp());
                        return Func_Name[Func_Name.size() - 1];
                }
                unsigned getBinaryPrecedence() const {return Precedence;}
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
        vector<llvm::Type*> Integers(Arguments.size(), llvm::Type::getInt32Ty(TheGlobalContext));
        llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheGlobalContext), Integers, false);
        llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Func_Name, Module_Ob);

        if(F->getName() != Func_Name)
        {
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
        Named_Values.clear();

        llvm::Function *TheFunction = Func_Decl->Codegen();
        if(TheFunction == 0)
                return 0;

        if (Func_Decl->isBinaryOp())
                Operator_Precedence[Func_Decl->getOperatorName()] = Func_Decl->getBinaryPrecedence();
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
static int next_token()
{
        //printff("Inside next_token():\n");
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
static BaseAST *unary_parser() {
        //printff("Inside unary_parser:\n");
        //BookError: !isascii->isascii
        if (isascii(Current_token) || Current_token == '(' || Current_token == ',')
                return Base_Parser();
        else {
                //printff("Not ret Base_parser.\n");
        }
        int Op = Current_token;
        next_token();
        //BookError ExprAST->BaseAST
        if (BaseAST *Operand = unary_parser())
                //BookError: Opc -> Op
                return new ExprUnaryAST(Op, Operand);
        return 0;
}
static BaseAST* expression_parser()
{
        //printff("Inside expression_parser:\n");
        BaseAST *LHS = unary_parser();
        if(!LHS)
                return 0;
        return binary_op_parser(0, LHS);
}
static BaseAST *If_parser()
{
        //printff("Inside If_parser:\n");
        next_token();

        BaseAST *Cond = expression_parser();
        if(!Cond)
                return 0;
        //printff("Done with Cond!\n");
        if(Current_token != THEN_TOKEN)
                return 0;
        next_token();
        BaseAST *Then = expression_parser();
        if(Then == 0)
                return 0;
        //printff("Done with Then!\n");
        if(Current_token != ELSE_TOKEN)
                return 0;
        next_token();
        BaseAST *Else = expression_parser();
        if(!Else)
                return 0;
        //printff("Done with Else!\n");
        return new ExprIfAST(Cond, Then, Else);
}
static BaseAST *For_parser()
{
        //printf("Inside For_parser:\n");
	next_token();
	if(Current_token != IDENTIFIER_TOKEN)
	 	return NULL;
	std::string IdName = Identifier_string;
	next_token();

	if(Current_token != '=')
		return NULL;

	next_token();
	BaseAST *Start = expression_parser();
	if(Start == 0)
		return NULL;
        //printf("Start Done!\n");
	if(Current_token != ',')
		return NULL;
	next_token();
	
	BaseAST *End = expression_parser();
	if(End == 0)
		return NULL;
	//printf("End Done!\n");
	BaseAST *Step = 0;
	if(Current_token == ','){
		next_token();
		Step = expression_parser();
		if(Step == 0)
			return 0;
	}
        //printf("Step Done!\n");
	if(Current_token != IN_TOKEN)
		return 0;
	next_token();

	BaseAST *Body = expression_parser();
	if(Body == 0)
		return 0;
	return new ExprForAST(IdName, Start, End, Step, Body);
}

static BaseAST* identifier_parser()
{
        //printff("Inside identifier_parser:\n");
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
        //printff("Inside Base_Parser:\n");
        switch(Current_token)
        {
                default: return 0;
                case IDENTIFIER_TOKEN : return identifier_parser();
                case NUMERIC_TOKEN : return numeric_parser();
                case '(' : return paran_parser();
                case IF_TOKEN : return If_parser();
                case FOR_TOKEN : return For_parser();
        }
}

static FunctionDeclAST *func_decl_parser()
{
        //printff("Inside func_decl_parser:\n");
        //printff("Current_token: %d\n", Current_token);
        std::string FnName = Identifier_string;

        unsigned Kind = 0;
        unsigned BinaryPrecedence = 30;

        switch (Current_token) {
                //printff("Inside Switch current token:\n");
                case IDENTIFIER_TOKEN:
                        //printff("case IDENTIFIER_TOKEN:\n");
                        FnName = Identifier_string;
                        Kind = 0;
                        next_token();
                        break;
                case UNARY_TOKEN:
                        //printff("case UNARY_TOKEN:\n");
                        next_token();
                        if (!isascii(Current_token))
                                return 0;
                        FnName = "unary";
                        FnName += (char)Current_token;
                        //printff("FnName: %s\n", FnName.c_str());
                        Kind = 1;
                        next_token();
                        break;
                case BINARY_TOKEN:
                        //printff("case BINARY_TOKEN:\n");
                        next_token();
                        if (!isascii(Current_token))
                                return 0;
                        FnName = "binary";
                        FnName += (char)Current_token;
                        Kind = 2;
                        next_token();

                        if (Current_token == NUMERIC_TOKEN) {
                                if (Numeric_Val < 1 || Numeric_Val > 100)
                                        return 0;
                                BinaryPrecedence = (unsigned)Numeric_Val;
                                next_token();
                        }
                        break;
                default: return 0;
        }

        if(Current_token != '(') {
                //printff("error in function declaration");
                return 0;
        }
                

        std::vector<std::string> Function_Argument_Names;

        while(next_token() == IDENTIFIER_TOKEN)
        {
                //printff("Identifier_string: %s \n", Identifier_string.c_str());
                Function_Argument_Names.push_back(Identifier_string);
        }

        if(Current_token != ')') {
            //printff("Expected ')'\n");
            return 0;
        }
                
        next_token();
        //printff("FnName: %s \n", FnName.c_str());
        if (Kind && Function_Argument_Names.size() != Kind)
                return 0;
        return new FunctionDeclAST(FnName, Function_Argument_Names, Kind != 0, BinaryPrecedence);
}
static FunctionDefnAST *func_defn_parser()
{
        //printff("Inside func_defn_parser:\n");
        next_token();
        //printff("Current_token: %d\n", Current_token);
        FunctionDeclAST *Decl = func_decl_parser();
        if(Decl == 0)
                return 0;
        //printff("===Func decl done!\n");
        if(BaseAST *Body = expression_parser())
        {
                return new FunctionDefnAST(Decl, Body);
        }
        return 0;
}

static int getBinOpPrecedence()
{
        //printff("Inside getBinOpPrecedence:\n");
        //printff("Current_token = %d\n", Current_token);

        //BookError: isascii->find==end
        if (Operator_Precedence.find(Current_token) == Operator_Precedence.end()) {
                return -1;
        }
        int TokPrec = Operator_Precedence[Current_token];
        //printff("Operator_Precedence = %d\n", TokPrec);
        //BookError: TokPrec<=0 ->TokPrec<0
        if(TokPrec < 0) return -1;

        return TokPrec;
}
static BaseAST* binary_op_parser(int Old_Prec, BaseAST *LHS)
{
        //printff("Inside binary_op_parser:\n");
        while(1)
        {
                int Operator_Prec = getBinOpPrecedence();
                //printff("Old_Prec = %d, Operator_Prec = %d \n", Old_Prec, Operator_Prec);
                if(Operator_Prec < Old_Prec)
                        return LHS;
                int BinOp = Current_token;
                next_token();

                BaseAST* RHS = unary_parser();
                if(!RHS) 
                        return 0;

                int Next_Prec = getBinOpPrecedence();
                if(Operator_Prec < Next_Prec)
                {
                        RHS = binary_op_parser(Operator_Prec + 1, RHS);
                        if(RHS == 0)
                                return 0;
                }
                LHS = new BinaryAST(std::to_string(BinOp), LHS, RHS);
        }
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
        //printff("Inside HandleDefn:\n");
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
        if (BaseAST *E = expression_parser()) {
                FunctionDeclAST *Func_Decl =
                        new FunctionDeclAST("", std::vector<std::string>());
                return new FunctionDefnAST(Func_Decl, E);
        }
        return 0;

}
static void HandleTopExpression()
{
        if(FunctionDefnAST *F = top_level_parser())
        {
                if(llvm::Function *LF = F ->Codegen())
                {
                        LF->dump();
                        void *FPtr = TheExeEngine->getPointerToFunction(LF);
                        int (*Int)()=(int (*)())(intptr_t)FPtr;
                        //printff("Evaluate to %d\n", Int());
                }
        }
        else
        {
                next_token();
        }
}
static void Driver()
{
        //printff("Inside Driver():\n");
        while(1)
        {
                //printff("Current_token: %d \n", Current_token);
                switch(Current_token)
                {       
                        case EOF_TOKEN: return;
                        case ';': next_token(); break;
                        case DEF_TOKEN: 
                                //printff("HandleDefn: \n");
                                HandleDefn(); 
                                break;
                        default: 
                                //printff("handleTopExpr: \n");
                                HandleTopExpression(); 
                                break;
                }
        }
}

int main(int argc, char *argv[])
{
        //printff("Inside main:\n");
        std::string err_str;
        //llvm::LLVMContext &Context = getGlobalContext();
        init_precedence();
        //add jit support
        
        file = fopen(argv[1], "r");
        if(file == 0)
        {
             printf("File not found.\n");
        }
        next_token();
        Module_Ob = new llvm::Module(argv[1], TheGlobalContext);
        llvm::FunctionPassManager FP(Module_Ob);
        llvm::EngineBuilder EB;

        TheExeEngine = EB.create();
        Global_FP = &FP;
        Driver();
        Module_Ob->dump();
        //Module_Ob->print(llvm::outs(), nullptr);
        return 0;
}
