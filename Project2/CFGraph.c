#include    <stdio.h>
#include    <stdlib.h>
#include    "util.h"
#include    "CFGraph.h"
#include    "GrammarCParser.h"

// функция для создания нового узла
CFGNode* createNode(ProcedureInfo *proc, myTreeNode* source, char* tag, OpNode* ops) {
    CFGNode* newNode = myAlloc(CFGNode);
    newNode->source = source;
    newNode->tag = tag;
    newNode->ops = ops;
    newNode->nextByDefault = NULL;
    newNode->nextByCondition = NULL;
    addLastLinkedListItem(proc->cfgNodes, newNode);
    return newNode;
}

//функция обхода дерева и построения графа потока управления
void buildCFG(myTreeNode* tree, CFGNode* graph, int* nodeCounter) {
    UNUSED(tree); UNUSED(graph); UNUSED(nodeCounter);
}

static TypeInfo* processTypeRef(myTreeNode* typeRefNode)
{
    TypeInfo* typeInfo = myAlloc(TypeInfo);
    typeInfo->kind = typeRefNode->children[0]->kind;
     //TODO
    return typeInfo;
}


static VarInfo* processVarInfo(myTreeNode* varNode)
{
    VarInfo* varInfo = myAlloc(VarInfo);

    if (varNode->childrenCount == 1) // no arg type
    {
        varInfo->type = NULL;
        varInfo->name = varNode->children[0]->text;
    }
    else
    {
        varInfo->type = processTypeRef(varNode->children[0]);
        varInfo->name = varNode->children[1]->text;
    }

    return varInfo;
}


typedef struct ModelCollectionState {
    ProgramModel *model;
    ProcedureInfo* currProcedure;
    SourceFileInfo* currentSourceFile;
} ModelCollectionState;


static OpNode* processExpression(myTreeNode* node, ModelCollectionState* s, bool isNotTopLevelExpr);


static OpNode* makeBinaryOperation(myTreeNode* node, OPKind op, ModelCollectionState* s)
{
    OpNode* o = myAllocWithArray(OpNode, OpNode*, 2);
    o->operands[0] = processExpression(node->children[0], s, false);
    o->operands[1] = processExpression(node->children[1], s, false);
    o->operandsCount = 2;
    o->kind = op;
    o->tag = NULL;
    return o;
}

static OpNode* makeUnaryOperation(myTreeNode* node, OPKind op, ModelCollectionState* s)
{
    OpNode* o = myAllocArray(OpNode, 1);
    o->operands[0] = processExpression(node->children[0], s, false);
    o->operandsCount = 1;
    o->kind = op;
    o->tag = NULL; 
    return o;

}


static OpNode *processExpression(myTreeNode* node, ModelCollectionState* s, bool isNotTopLevelExpr)
{
    switch (node->kind)
    {
    case AST_EXPR_DIV: return makeBinaryOperation(node, OP_DIV, s);
    case AST_EXPR_MUL: return makeBinaryOperation(node, OP_MUL, s);
    case AST_EXPR_SUB: return makeBinaryOperation(node, OP_SUB, s);
    case AST_EXPR_SUM: return makeBinaryOperation(node, OP_SUM, s);
    case AST_EXPR_NOT_EQUAL: return makeBinaryOperation(node, OP_NOT_EQUAL, s);
    case AST_EXPR_LESS: return makeBinaryOperation(node, OP_LESS, s);
    case AST_EXPR_GREATER: return makeBinaryOperation(node, OP_GREATER, s);
    case AST_EXPR_LESS_EQUAL: return makeBinaryOperation(node, OP_LESS_EQUAL, s);
    case AST_EXPR_GREATER_EQUAL: return makeBinaryOperation(node, OP_GREATER_EQUAL, s); 

    case AST_EXPR_INV: return makeBinaryOperation(node, OP_INV, s);
    case AST_EXPR_NEG: return makeBinaryOperation(node, OP_NEG, s);
    case AST_EXPR_NOT: return makeBinaryOperation(node, OP_NOT, s);

    case AST_CALL_OR_INDEXER_EXPR: {
        OpNode* o = myAllocWithArray(OpNode, OpNode*, 2);
        o->tag = node->children[0]->text;
        o->operandsCount = 2;
        o->operands[0] = processExpression(node->children[1], s, true);
        o->operands[1] = processExpression(node->children[1], s, true);
        o->kind = OP_ARR_GET_ITEM;
        return o;
    }
    case STR:
    case CHAR:
    case BITSCONST:
    case BOOL: 
    case INT: { 
        OpNode* o = myAllocWithArray(OpNode, OpNode*, 0); 
        o->tag = node->text; 
        o->operandsCount = 0; 
        o->kind = OP_CONST; 
        return o;
    }
    case HEXCONST: { 
        OpNode* o = myAllocWithArray(OpNode, OpNode*, 0);
        o->tag = node->text;
        o->operandsCount = 0;
        o->kind = OP_CONST;
        return o;
    }
    case AST_ID: { // op_get_var
        OpNode* o = myAllocWithArray(OpNode, OpNode*, 1);
        o->tag = node->children[0]->text;
        o->operandsCount = 0;
        o->kind = OP_GET_VAR;
        return o;
    } 
    case AST_EXPR_EQUAL: {
        if (isNotTopLevelExpr) {
            return makeBinaryOperation(node, OP_EQUAL, s);
        }
        else {
            if (node->children[0]->children[0]->kind == AST_ID) {
                OpNode* o = myAllocWithArray(OpNode, OpNode*, 1);
                o->tag = node->children[0]->children[0]->children[0]->text;
                o->operandsCount = 2;
                o->operands[0] = processExpression(node->children[0]->children[0], s, true);
                o->operands[1] = processExpression(node->children[0]->children[1], s, true);
                o->kind = OP_SET_VAR;
                return o;
            }
            else if (node->children[0]->children[0]->kind == AST_CALL_OR_INDEXER_EXPR) {
                myTreeNode* indexerNode = node->children[0];
                if (indexerNode->children[0]->kind == AST_ID) {
                    OpNode* o = myAllocWithArray(OpNode, OpNode*, 2);
                    o->tag = indexerNode->children[0]->text;
                    o->operandsCount = 2;
                    o->operands[0] = processExpression(indexerNode->children[1], s, true);
                    o->operands[1] = processExpression(node->children[0]->children[1], s, true);
                    o->kind = OP_ARR_SET_ITEM;
                    return o;
                }
                else {
                    // log errror cannot apply indexer for non-value place expression
                }
                
            }
            else {
                // log errror cannot assign to non-value place target

            }
        }
    }

    // TODO

    default: {
        OpNode* o = myAllocArray(OpNode, 0);
        o->kind = OP_ERROR;
        o->tag = NULL;
        o->operandsCount = 0;
        // TODO collect error info
        return o;
    }
    }
}

static CFGNode* processStatement(CFGNode* cn, myTreeNode* node, ModelCollectionState* s)
{
    switch (node->kind)
    {
    case IF_STMT: {
        CFGNode* exit = createNode(s->currProcedure, node, "if-leave", NULL);
        CFGNode* condition = createNode(s->currProcedure, node, "if-cond", processExpression(node->children[0], s, false ));
        cn->nextByDefault = condition;
        CFGNode* thenBody = createNode(s->currProcedure, node, "if-then", NULL);
        condition->nextByCondition = thenBody;
        CFGNode* thenExit = createNode(s->currProcedure, node, "if-leave-then", NULL);
        CFGNode* thenBodyTail = processStatement(thenBody, node->children[1], s);
        thenBodyTail->nextByDefault = thenExit;
        thenExit->nextByDefault = exit;
        if (node->childrenCount > 2)
        {
            CFGNode* elseBody = createNode(s->currProcedure, node, "if-else", NULL);
            condition->nextByDefault = elseBody;
            CFGNode* elseBodyTail = processStatement(elseBody, node->children[2], s);
            CFGNode* elseExit = createNode(s->currProcedure, node, "if-else-leave", NULL);
            elseBodyTail->nextByDefault = elseExit;
            elseExit->nextByDefault = exit;
        }
        else 
        {
            condition->nextByDefault = exit;
        }
        return exit;
    }   
    case AST_BLOCK: {
        for (int i = 0; i < node->childrenCount; i++) {
            cn = processStatement(cn, node->children[i], s);
        }
        return cn;
    }
    case DO_STMT: {
        CFGNode* enterBody = createNode(s->currProcedure, node, "do-enter", NULL);
        cn->nextByDefault = enterBody;
        CFGNode* exitBody = createNode(s->currProcedure, node, "do-cond", processExpression(node->children[1], s, false));
        CFGNode* exit = createNode(s->currProcedure, node, "do-leave", NULL);
        exitBody->nextByCondition = enterBody;
        exitBody->nextByDefault = exit;
        CFGNode* loopBodyTail = processStatement(enterBody, node->children[2], s);
        loopBodyTail->nextByDefault = exitBody;
        return exit;
    }
    case WHILE_STMT: {
        CFGNode* loopHead = createNode(s->currProcedure, node, "while-cond", processExpression(node->children[0], s, false));
        CFGNode* enterLoop = createNode(s->currProcedure, node, "while-enter-body", NULL);
        CFGNode* exitLoop = createNode(s->currProcedure, node, "while-leave-body", NULL);
        loopHead->nextByCondition = enterLoop;
        loopHead->nextByDefault = exitLoop;
        cn->nextByDefault = loopHead;
        CFGNode* loopBodyTail = processStatement(enterLoop, node->children[1], s);
        loopBodyTail->nextByDefault= loopHead;
        return exitLoop;
    }
    case AST_STMT_EXPR: {
        CFGNode* exprStat = createNode(s->currProcedure, node, "expr", processExpression(node->children[0], s, false));
        cn->nextByDefault = exprStat;
        return exprStat;
    }
    case BREAK_STMT: {
        CFGNode* breakStat = createNode(s->currProcedure, node, "break", NULL);
        cn->nextByDefault = breakStat;
        return breakStat;
    }
    case VAR_STMT: {
        // remeber that variable with the given name exists in the procedure
        CFGNode* varStat = createNode(s->currProcedure, node, "var", NULL);
        cn->nextByDefault = varStat;
        return varStat;
    }
    default: {
        // TODO collect error info
        return cn;
    }
    }
}

static void processProcedure(myTreeNode *fdecl, ModelCollectionState* s)
{
    ProcedureInfo* p = myAlloc(ProcedureInfo);
    p->name = fdecl->children[0]->children[0]->children[0]->text;
    p->cfgNodes = createLinkedList();
    p->cfgStart = NULL;
    p->scope = NULL;
    p->sourceFile = s->currentSourceFile;
 
    if (fdecl->children[0]->childrenCount == 1) // no args, no ret type
    {
        p->signature = myAllocWithArray(SignInfo, VarInfo, 0);
        p->signature->resulType = NULL;
        p->signature->argsCount = 0;
    }
    else 
    {
        int hasRetType = fdecl->children[0]->children[1]->kind == AST_TYPE_REF;
        int argsCount = fdecl->children[0]->childrenCount - (hasRetType ? 2 : 1);
        p->signature = myAllocWithArray(SignInfo, VarInfo, argsCount);
        p->signature->argsCount = argsCount;
        p->signature->resulType = hasRetType ? processTypeRef(fdecl->children[0]->children[1]) : NULL;

        for (int i = (hasRetType ? 2 : 1), j = 0; i < fdecl->children[0]->childrenCount; i++, j++)
        {
            p->signature->args[j] = processVarInfo(fdecl->children[0]->children[i]);
        }
    }

    tryAddRbTreeItem(s->model->procsByName, p->name, p);

    int varsCount = fdecl->children[1]->childrenCount;
    p->scope = myAlloc(VarsScopeInfo);
    p->scope->varsList = createLinkedList();
    //p->scope->vars[] = ;

    s->currProcedure = p;

    CFGNode* startNode = createNode(p, fdecl->children[1], p->name, NULL);
    p->cfgStart = startNode;
    processStatement(startNode, fdecl->children[1], s);
 
    s->currProcedure = NULL;
}

static void processSourceAst(SourceFileInfo *f, ModelCollectionState *s)
{
    s->currentSourceFile = f;
    for (int i = 0; i < f->ast->childrenCount; i++)
    {
        processProcedure(f->ast->children[i], s);
    }
}

ProgramModel *prepareModel(SourcesInfo* sources)
{
    ProgramModel* model = myAlloc(ProgramModel);
    model->procsByName = createRbTreeByString();

    ModelCollectionState s = {
        .model = model
    };

    traverseLinkedList(sources->sourceFiles, processSourceAst, &s);


    return model;
}
