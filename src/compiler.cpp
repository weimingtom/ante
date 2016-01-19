#include "compiler.h"
#include "parser.h"

using namespace llvm;

/*
 *  Translates an individual type in token form to an llvm::Type
 */
Type* translateType(int tokTy, string typeName = "")
{
    switch(tokTy){
        case Tok_UserType: //TODO: implement
            return Type::getVoidTy(getGlobalContext());
        case Tok_I8:  case Tok_U8:  return Type::getInt8Ty(getGlobalContext());
        case Tok_I16: case Tok_U16: return Type::getInt16Ty(getGlobalContext());
        case Tok_I32: case Tok_U32: return Type::getInt32Ty(getGlobalContext());
        case Tok_I64: case Tok_U64: return Type::getInt64Ty(getGlobalContext());
        case Tok_Isz: return Type::getVoidTy(getGlobalContext()); //TODO: implement
        case Tok_Usz: return Type::getVoidTy(getGlobalContext()); //TODO: implement
        case Tok_F32: return Type::getFloatTy(getGlobalContext());
        case Tok_F64: return Type::getDoubleTy(getGlobalContext());
        case Tok_C8:  return Type::getVoidTy(getGlobalContext()); //TODO: implement
        case Tok_C32: return Type::getVoidTy(getGlobalContext()); //TODO: implement
        case Tok_Bool:return Type::getInt1Ty(getGlobalContext());
        case Tok_Void:return Type::getVoidTy(getGlobalContext());
    }
    return nullptr;
}

void compileStmtList(Node *nList, Compiler *c, Module *m)
{
    c->enterNewScope();
    while(nList){
        puts("stmt");
        nList->compile(c, m);
        nList = nList->next.get();
    }
    c->exitScope();
}

Value* IntLitNode::compile(Compiler *c, Module *m)
{   //TODO: unsigned int with APUInt
    return ConstantInt::get(getGlobalContext(), APInt(64, val, true));
}

Value* FltLitNode::compile(Compiler *c, Module *m)
{
    return ConstantFP::get(getGlobalContext(), APFloat(APFloat::IEEEquad, val));
}

Value* BoolLitNode::compile(Compiler *c, Module *m)
{
    return ConstantInt::get(getGlobalContext(), APInt(1, val, true));
}

Value* TypeNode::compile(Compiler *c, Module *m)
{ return nullptr; }

Value* StrLitNode::compile(Compiler *c, Module *m)
{ return nullptr; }

/*
 *  Compiles an operation along with its lhs and rhs
 *
 *  TODO: type checking
 *  TODO: CreateExactUDiv for when it is known there is no remainder
 *  TODO: CreateFcmpOEQ vs CreateFCmpUEQ
 */
Value* BinOpNode::compile(Compiler *c, Module *m)
{
    Value *lhs = lval->compile(c, m);
    puts("lhs done");
    Value *rhs = rval->compile(c, m);
    puts("rhs done");

    switch(op){
        case '+': return c->builder.CreateAdd(lhs, rhs, "fAddTmp");
        case '-': return c->builder.CreateSub(lhs, rhs, "fSubTmp");
        case '*': return c->builder.CreateMul(lhs, rhs, "fMulTmp");
        case '/': return c->builder.CreateSDiv(lhs, rhs, "fDivTmp");
        case '%': return c->builder.CreateSRem(lhs, rhs, "fModTmp");
        case '<': return c->builder.CreateICmpULT(lhs, rhs, "fLtTmp");
        case '>': return c->builder.CreateICmpUGT(lhs, rhs, "fGtTmp");
        case '^': return c->builder.CreateXor(lhs, rhs, "xorTmp");
        case '.': break;
        case Tok_Eq: return c->builder.CreateICmpEQ(lhs, rhs, "fCmpEqTmp");
        case Tok_NotEq: return c->builder.CreateICmpNE(lhs, rhs, "fCmpNeTmp");
        case Tok_LesrEq: return c->builder.CreateICmpULE(lhs, rhs, "fLeTmp");
        case Tok_GrtrEq: return c->builder.CreateICmpUGE(lhs, rhs, "fGeTmp");
        case Tok_Or: break;
        case Tok_And: break;
    }
   
    cout << "Warning: unknown operator ";
    lexer::printTok(op);
    return nullptr;
}

Value* RetNode::compile(Compiler *c, Module *m)
{
    return c->builder.CreateRet(expr->compile(c, m));
}

Value* IfNode::compile(Compiler *c, Module *m)
{ return nullptr; }

Value* NamedValNode::compile(Compiler *c, Module *m)
{ return nullptr; }

/*
 *  Loads a variable from the stack
 */
Value* VarNode::compile(Compiler *c, Module *m)
{ //TODO: check for var not declared
    //Value *v = c->lookup(name);
    printf("%p\n", (void*)c->varTable.top()[name]);
    //Value *r = c->builder.CreateLoad(v, name.c_str());
    //return r;
    return c->varTable.top()[name];
}

Value* FuncCallNode::compile(Compiler *c, Module *m)
{ return nullptr; }

Value* VarDeclNode::compile(Compiler *c, Module *m)
{ return nullptr; }

Value* VarAssignNode::compile(Compiler *c, Module *m)
{ return nullptr; }


Value* FuncDeclNode::compile(Compiler *c, Module *m)
{
    //vector<llvm::Type*> paramTypes{2, Type::getDoubleTy(getGlobalContext())};
    TypeNode *retNode = (TypeNode*)type.get();
    Type *retType = translateType(retNode->type, retNode->typeName);

    TypeNode *paramTyNode = (TypeNode*)params.get()->typeExpr.get();
    Type *paramsType = translateType(paramTyNode->type, paramTyNode->typeName);

    FunctionType *ft = FunctionType::get(retType, paramsType, false);
    Function *f = Function::Create(ft, Function::ExternalLinkage, name, m);

    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", f);
    c->builder.SetInsertPoint(bb);

    for(auto &arg : f->args()){
        //AllocaInst *v = c->builder.CreateAlloca(translateType(((TypeNode*)param->typeExpr.get())->type), 0, param->name);
        c->varTable.top()[name] = (AllocaInst*)&arg;
    }

    compileStmtList(child.get(), c, m);

    if(retNode->type == Tok_Void){
        c->builder.CreateRetVoid();
    }
    
    puts("function: ");
    m->dump();
    puts("endFunction.");

    verifyFunction(*f);
    return f;
}


Value* DataDeclNode::compile(Compiler *c, Module *m)
{ return nullptr; }



void IntLitNode::exec(){}

void FltLitNode::exec(){}

void BoolLitNode::exec(){}

void TypeNode::exec(){}

void StrLitNode::exec(){}

void BinOpNode::exec(){}

void RetNode::exec(){}

void IfNode::exec(){}

void VarNode::exec(){}

void NamedValNode::exec(){}

void FuncCallNode::exec(){}

void VarDeclNode::exec(){}

void VarAssignNode::exec(){}

void FuncDeclNode::exec(){}

void DataDeclNode::exec(){}



void IntLitNode::print()
{
    cout << val;
}

void FltLitNode::print()
{
    cout << val;
}

void BoolLitNode::print()
{
    if(val)
        cout << "true";
    else
        cout << "false";
}

void StrLitNode::print()
{
    cout << '"' << val << '"';
}

void TypeNode::print()
{
    if(type == Tok_Ident || type == Tok_UserType){
        cout << "Type: " << typeName;
    }else{
        cout << "Type: ";
        ante::lexer::printTok(type);
    }
}

void BinOpNode::print()
{
    putchar('(');
    if(lval) lval->print();
    putchar(' ');
    if(IS_LITERAL(op))
        cout << (char)op;
    else
        cout << TOK_TYPE_STR(op);
    putchar(' ');
    if(rval) rval->print();
    puts(")");
}

void RetNode::print()
{
    cout << "return ";
    if(expr) expr->print();
    putchar('\n');
}

void IfNode::print()
{
    cout << "if ";
    if(condition) condition->print();
    cout << "\nthen\n";
    if(child) child->print();
    cout << "EndIf\n";
}

void NamedValNode::print()
{
    cout << "{NamedValNode " << name << '}';
}

void VarNode::print()
{
    cout << name;
}

void FuncCallNode::print()
{
    cout << "fnCall " << name << " called with params (";
    if(params) params->print();
    cout << ")\n";
}

void VarDeclNode::print()
{
    cout << "varDecl " << name << " = ";
    if(expr) expr->print();
    else cout << "(undef)";
    putchar('\n');
}

void VarAssignNode::print()
{
    cout << "varAssign ";
    if(var) var->print();
    cout << " = ";
    if(expr) expr->print();
    else cout << "(undef)";
    putchar('\n');
}

void FuncDeclNode::print()
{
    cout << "function " << name << " declared of ";
    type->print();
    puts("With params: ");
    if(params) params->print();
    puts("FuncBody:");
    if(child.get()) child.get()->print();
    puts("EndFunc");
}

void DataDeclNode::print()
{
    cout << "Data " << name << "Declared\n";
    if(child.get()) child.get()->print();
    puts("");
}



void Compiler::compile()
{
    Node *n = ast.get();
    while(n){
        n->compile(this, module.get());
        n = n->next.get();
    }
    module->dump();
}

void Compiler::enterNewScope()
{
    varTable.push(map<string, AllocaInst*>());
}

void Compiler::exitScope()
{
    varTable.pop();
}

Value* Compiler::lookup(string var)
{
    return varTable.top()[var];
}

/*
 *  Allocates a value on the stack at the entry to a block
 */
/*static AllocaInst* createBlockAlloca(Function *f, string var, Type *varType)
{
    IRBuilder<> builder{&f->getEntryBlock(), f->getEntryBlock().begin()};
    return builder.CreateAlloca(varType, 0, var);
}*/
