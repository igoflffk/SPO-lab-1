#pragma once

#include "parsing.h"
#include "collections.h"


typedef struct SourceFileInfo {
    sourceFileText text;
    myTreeNode* ast;
} SourceFileInfo;


typedef enum OPKind {
    OP_ERROR,
    OP_AND,
    OP_OR,
    OP_LESS,
    OP_GREATER,
    OP_LESS_EQUAL,
    OP_GREATER_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_DIV,
    OP_MUL,
    OP_SUB,
    OP_SUM,
    OP_INV,
    OP_NEG,
    OP_NOT,
    OP_CONST,
    OP_ARR_SLICE,
    OP_ARR_NEW,
    OP_ARR_GET_ITEM,
    OP_ARR_SET_ITEM,
    OP_CALL,
    OP_GET_VAR,
    OP_SET_VAR


}OPKind;

typedef enum TypeKind {

    TK_BOOL,
    TK_BYTE,
    TK_INT,
    TK_UINT,
    TK_LONG,
    TK_ULONG,
    TK_CHAR,
    TK_STRING,
    TK_CUSTOM,
    TK_ARRAY

}TypeKind;

typedef struct CFGNode {
    struct CFGNode* nextByDefault;
    struct CFGNode* nextByCondition;
    struct OpNode* ops;
    char *tag;
    myTreeNode* source;
} CFGNode;

typedef struct OpNode {
    OPKind kind;
    char* tag;
    int operandsCount;
    struct OpNode* operands[];
} OpNode;

typedef struct TypeInfo {
    TypeKind kind;
    union {
        struct {
            char* name;
        }custom;
        struct {
         int demensions;
         struct TypeInfo* elementType;
        }array;
        
    }extra;
}TypeInfo;


typedef struct VarInfo {
    char* name;
    TypeInfo* type;
}VarInfo;

typedef struct VarsScopeInfo {
    LinkedList* varsList; // list of VarInfo
}VarsScopeInfo;

typedef struct SignInfo {
    TypeInfo* resulType;
    int argsCount;
    VarInfo* args[];
}SignInfo;

typedef struct ProcedureInfo {
    char* name;
    SignInfo *signature;
    VarsScopeInfo *scope;
    CFGNode *cfgStart;
    LinkedList* cfgNodes;
    SourceFileInfo* sourceFile;
} ProcedureInfo;

typedef struct ProgramModel {
    RbTree* procsByName;
} ProgramModel;

typedef struct SourcesInfo {
    LinkedList* sourceFiles;
} SourcesInfo;

// void buildCFG(myTreeNode *tree, CFGNode* graph, int* nodeCounter);
ProgramModel *prepareModel(SourcesInfo* sources);

