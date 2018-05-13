/*--------------------------------
*   * The lexer and AST and parser
*    --------------------------- */
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
using namespace std;
enum Token_Type {
        EOF_TOKEN = 0,
        NUMERIC_TOKEN,    //current token is of numeric type
        IDENTIFIER_TOKEN, //identifier
        PARAN_TOKEN,      //parenthesis
        DEF_TOKEN         //funciton definition
};
static int Numeric_Val;
static std::string Identifier_string;
FILE *file;
static int get_token()
{
        static int LastChar = ' ';

        while (isspace(LastChar))
                LastChar = fgetc(file);
        if (isalpha(LastChar))
        {
                Identifier_string = LastChar;

                while (isalnum(LastChar = fgetc(file)))
                        Identifier_string += LastChar;

                if (Identifier_string == "def")
                        return DEF_TOKEN;

                return IDENTIFIER_TOKEN;
        }

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
                return NUMERIC_TOKEN;
        }
        //comment
        if(LastChar == '#')
        {
                do{
                        LastChar = fgetc(file);
                }while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

                if(LastChar != EOF)
                        return get_token();
        }
        if(LastChar == EOF)
                return EOF_TOKEN;
        
        int ThisChar = LastChar;
        LastChar = fgetc(file);
        return ThisChar;
}
//AST for different types
class BaseAST
{
        public:
        virtual ~BaseAST();
};
//AST class for variable expressions, name matters most
class VariableAST : public BaseAST
{
        std::string Var_Name;
        public:
                VariableAST(std::string &name) : Var_Name(name){}
};
//for numeric expressions: value matters
class NumericAST : public BaseAST
{
        int numeric_val;
        public:
                NumericAST(int val) : numeric_val(val) {}
};
//binary operation expressions: op and two operands matter
class BinaryAST : public BaseAST
{
        std::string Bin_Operator;
        BaseAST *LHS, *RHS;
        public:
        BinaryAST(std::string op, BaseAST *lhs, BaseAST *rhs): 
                Bin_Operator(op), LHS(lhs), RHS(rhs) {}
};
//Function declaration: name and arg vector, why no ret value.
class FunctionDeclAST
{
        std::string Func_Name;
        std::vector<std::string> Arguments;
        public:
                FunctionDeclAST(const std::string &name, const std::vector<std::string> &args):
                        Func_Name(name), Arguments(args) {}
};
//Function definition: declaration (name and arg vector) and body.
class FunctionDefnAST
{
        FunctionDeclAST *Func_Decl;
        BaseAST* Body;
        public:
                FunctionDefnAST(FunctionDeclAST *proto, BaseAST *body) : Func_Decl(proto), Body(body){}
};
//Function call: name of callee and function arguments
class FunctionCallAST : public BaseAST
{
        std::string Function_Callee;
        std::vector<BaseAST *> Function_Arguments;
        public:
                FunctionCallAST(const std::string &Callee, std::vector<BaseAST*> &args):
                        Function_Callee(Callee), Function_Arguments(args){}
};
/*-----------
* Parser
-----------*/
//store in the Current token 
static int Current_token;
static int next_token()
{
        Current_token = get_token();
        return Current_token;
}
static BaseAST* paran_parser();
static BaseAST *numeric_parser()
{
        BaseAST *Result = new NumericAST(Numeric_Val);
        next_token();
        return Result;
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
        switch(Current_token)
        {
                default: return 0;
                case IDENTIFIER_TOKEN : return identifier_parser();
                case NUMERIC_TOKEN : return numeric_parser();
                case '(' : return paran_parser();
        }
}
static BaseAST* binary_op_parser(int, BaseAST *);
static BaseAST* expression_parser()
{
        BaseAST *LHS = Base_Parser();
        if(!LHS)
                return 0;
        return binary_op_parser(0, LHS);
}
static FunctionDeclAST *func_decl_parser()
{
        if(Current_token != IDENTIFIER_TOKEN)
                return 0;
        
        std::string FnName = Identifier_string;
        next_token();

        if(Current_token != '(')
                return 0;

        std::vector<std::string> Function_Argument_Names;

        while(next_token() == IDENTIFIER_TOKEN)
        {
                Function_Argument_Names.push_back(Identifier_string);
        }

        if(Current_token != ')')
                return 0;

        return new FunctionDeclAST(FnName, Function_Argument_Names);
}
static FunctionDefnAST *func_defn_parser()
{
        next_token();
        FunctionDeclAST *Decl = func_decl_parser();
        if(Decl == 0)
                return 0;
        
        if(BaseAST *Body = expression_parser())
        {
                return new FunctionDefnAST(Decl, Body);
        }
        return 0;
}


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
        while(1)
        {
                int Operator_Prec = getBinOpPrecedence();
                if(Operator_Prec < Old_Prec)
                        return LHS;
                int BinOp = Current_token;
                next_token();

                BaseAST* RHS = Base_Parser();
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
static void HandleTopExpression()
{
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
static void Driver()
{
        while(1)
        {
                switch(Current_token)
                {       
                        case EOF_TOKEN: return;
                        case ';': next_token(); break;
                        case DEF_TOKEN: HandleDefn(); break;
                        default: HandleTopExpression(); break;
                }
        }
}
int main(int argc, char *argv[])
{
        llvm::LLVMContext &Context = getGlobalContext();
        init_precedence();
        file = fopen(argv[1], "r");
        if(file == 0)
        {
                printf("File not found.\n");
        }
        next_token();
        llvm::Module *Module_Ob = new llvm::Module("TOY Compiler", Context);
        Driver();
        Module_Ob->dump();
        return 0;
}