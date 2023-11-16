#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
//#include    "stdio.h"
// #include    "GrammarCWalker.h"

#include "parsing.h"
#include "CFGraph.h"

#pragma warning( disable : 4996)


bool tryReadFile(char* fileName, sourceFileText* result)
{
    FILE* f = fopen(fileName, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        long fileLength = ftell(f);
        fseek(f, 0, SEEK_SET);

        char* text = malloc(fileLength);
        if (text)
        {
            fread(text, fileLength, 1, f);
            fclose(f);

            result->fileName = fileName;
            result->length = fileLength;
            result->text = text;
            return true;
        }
        else
        {
            fprintf(stderr, "Failed to allocate %d bytes of memory while reading file %s ", fileLength, fileName);
            fclose(f);
            return false;
        }
    }
    else
    {
        fprintf(stderr, "Failed to open file %s", fileName);
        perror(NULL);
        return false;
    }
}

SourcesInfo* parseSourceFiles(int count, char** names)
{
    SourcesInfo* sources = myAlloc(SourcesInfo);
    sources->sourceFiles = createLinkedList();

    for (int i = 0; i < count; i++)
    {
        sourceFileText sourceFile;
        if (tryReadFile(names[i], &sourceFile))
        {
            myTreeNode* ast = parseText(&sourceFile);

            if (ast)
            {
                SourceFileInfo* sourceInfo = myAlloc(SourceFileInfo);
                sourceInfo->text = sourceFile;
                sourceInfo->ast = ast;
                
                formatAstAsDgmlToFile("tree.dgml", ast);

                addLastLinkedListItem(sources->sourceFiles, sourceInfo);
            }
        }
    }

    return sources;
}

typedef struct CfgsFormattingState {
    char* outDirName;
    FILE* currFile;
} CfgsFormattingState;

typedef union CfgNodePtr {
    CFGNode* ptr;
    unsigned long int value;
} CfgNodePtr;


static char*  getOpNodeKindName(OpNode* op) {
    switch (op->kind) {
    case OP_ERROR: return "Error";
    case OP_AND: return "AND";
    case OP_OR: return "OR";
    case OP_GREATER: return "OP_GREATER";
    case OP_LESS: return "OP_LESS";
    case OP_LESS_EQUAL: return "OP_LESS_EQUAL";
    case OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
    case OP_EQUAL: return "OP_EQUAL";
    case OP_NOT_EQUAL: return "OP_NOT_EQUAL";
    case OP_DIV: return "OP_DIV";
    case OP_MUL: return "OP_MUL";
    case OP_SUB: return "OP_SUB";
    case OP_SUM: return "OP_SUM";
    case OP_INV: return "OP_INV";
    case OP_NEG: return "OP_NEG";
    case OP_NOT: return "OP_NOT";
    case OP_CONST: return "OP_CONST";
    case OP_ARR_GET_ITEM: return "OP_ARR_GET_ITEM";
    case OP_ARR_SET_ITEM: return "OP_ARR_SET_ITEM";
    case OP_CALL: return "OP_CALL";
    case OP_GET_VAR: return "OP_GET_VAR";
    case OP_SET_VAR: return "OP_SET_VAR";
    default: {return "default"; }
    }
}


static void formatOpNode(FILE* file, OpNode* op)
{
    fprintf(file, "%s[%s](", getOpNodeKindName(op), op->tag ? op->tag : "");
    for (int i = 0; i < op->operandsCount; i++) {
        if (i > 0) {
            fprintf(file, ", ");
        }
        formatOpNode(file, op->operands[i]);
    }
    fprintf(file, ")");
}
void formatCfgNode(CFGNode* node, CfgsFormattingState* state)
{
    CfgNodePtr ptr = { .ptr = node };
    fprintf(state->currFile, "\t\t<Node Id=\"#%x\" Label=\"[#%d] #%x #%s #%d&#10;", ptr.value, node->source == NULL ? -1 : node->source->id, ptr.value, node->tag ? node->tag : "", node->source->childrenCount /*,node->ops->operands*/);
    if (node->ops) {
        formatOpNode(state->currFile, node->ops); // traverse opnodes
    }
    fprintf(state->currFile, "\"/>\n");
}
void formatCfgLink(CFGNode* node, CfgsFormattingState* state)
{
    CfgNodePtr ptr = { .ptr = node };
    if (node->nextByDefault)
    {
        CfgNodePtr nextPtr = { .ptr = node->nextByDefault };
        fprintf(state->currFile, "\t\t<Link Source=\"#%x\" Target=\"#%x\" />\n", ptr.value, nextPtr.value);
    }

    if (node->nextByCondition)
    {
        CfgNodePtr nextPtr = { .ptr = node->nextByCondition };
        fprintf(state->currFile, "\t\t<Link Source=\"#%x\" Target=\"#%x\" />\n", ptr.value, nextPtr.value);
    }
}

char* extractFileName(char* path)
{
    char* fnp1 = strrchr(path, '/');
    char* fnp2 = strrchr(path, '\\');
    char* sourceName = max(path, max(fnp1, fnp2) + 1);
    char* extp = strrchr(sourceName, '.');
    return substring(sourceName, 0, (int)(extp - sourceName));
}

void formatCfgToOutputDir(char* name, ProcedureInfo* info, CfgsFormattingState* state)
{
    char* sourceBaseName = extractFileName(info->sourceFile->text.fileName);
    char* fname = formatString("%s.%s.dgml", sourceBaseName, name);
    state->currFile = fopen(fname, "w+");
    
    // printf dgml head
    fprintf(state->currFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(state->currFile, "<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\">\n");
    fprintf(state->currFile, "\t<Nodes>\n");
    traverseLinkedList(info->cfgNodes, &formatCfgNode, state);

    // printf dgml mid
    fprintf(state->currFile, "\t</Nodes>\n");
    fprintf(state->currFile, "\t<Links>\n");
    traverseLinkedList(info->cfgNodes, &formatCfgLink, state);

    // printf dgml tail
    fprintf(state->currFile, "\t</Links>\n");
    fprintf(state->currFile, "</DirectedGraph>\n");
    //fclose(file);
    fclose(state->currFile);

    free(fname);
    free(sourceBaseName);
}

int main(int argc, char* argv[])
{
    if (argc < 3 || argv[1] == NULL)
    {
        fprintf(stderr, "Not enough arguments");
        return -2;
    }
    else
    {
        // parseSourceFiles(argc - 2, &argv[1]);

        SourcesInfo* sources = parseSourceFiles(argc - 2, &argv[1]);
        
        ProgramModel* model = prepareModel(sources);

        CfgsFormattingState s = {
            .outDirName = argv[argc - 1]
        };

        traverseRbTree(model->procsByName, &formatCfgToOutputDir, &s);

        return 0;
    }
}