
#include <stdio.h>

#include "GrammarCLexer.h"
#include "GrammarCParser.h"
#include "parsing.h"



typedef struct antlrTreeTranslationState {
    int idCounter;
} antlrTreeTranslationState;

static char* cloneNodeText(pANTLR3_BASE_TREE node)
{
    pANTLR3_STRING str = node->toString(node);
    char* text = malloc(sizeof(char) * str->len + 1);
    strcpy(text, (const char*)str->chars);
    return text;
}

static myTreeNode* translateAntlrTreeImpl(pANTLR3_BASE_TREE node, antlrTreeTranslationState* state)
{
    int childrenCount = node->getChildCount(node);

    myTreeNode* curr = (myTreeNode*)malloc(sizeof(myTreeNode));
    curr->id = state->idCounter++;
    curr->kind = node->getType(node);
    curr->text = cloneNodeText(node);
    curr->children = (myTreeNode**)malloc(sizeof(myTreeNode*) * childrenCount);
    curr->childrenCount = childrenCount;

    //printf("Line: %d  ", node->getToken(node)->line);
    //printf("Type: %d  ", node->getType(node));
    //printf("Id:   %s\n ", node->toString(node)->chars);

    for (int i = 0; i < childrenCount; i++) {
        curr->children[i] = translateAntlrTreeImpl(node->getChild(node, i), state);
    }

    return curr;
}

static myTreeNode* translateAntlrTree(pANTLR3_BASE_TREE node)
{
    antlrTreeTranslationState state = { 0 };
    return translateAntlrTreeImpl(node, &state);
}


static void traverseTree(pANTLR3_BASE_TREE node, int depth)
{
    for (int i = 0; i < depth; i++) {
        printf("+-->");
    }
    // node->getToken(node)->line

    printf("%d:%s\n", node->getType(node), node->toString(node)->chars);

    for (unsigned int i = 0; i < node->getChildCount(node); i++) {
        traverseTree(node->getChild(node, i), depth + 1);
    }
}


static void generateDGMLforNode(FILE* file, myTreeNode* node)
{
    fprintf(file, "\t\t<Node Id = \" %d \" Label = \"[#%d] %s\"/>\n", node->id, node->id, node->text);
    for (int i = 0; i < node->childrenCount; i++) {
        generateDGMLforNode(file, node->children[i]);
    }
}

static void generateDGMLforLinks(FILE* file, myTreeNode* node)
{
    for (int i = 0; i < node->childrenCount; i++) {
        fprintf(file, "\t\t<Link Source=\" %d \" Target=\" %d \" Label=\"%d\" />\n", node->id, node->children[i]->id, i);
        generateDGMLforLinks(file, node->children[i]);
    }
}


void formatAstAsDgmlToFile(const char* outputFile, myTreeNode* tree) {

    FILE* file = fopen(outputFile, "w+");

    if (file != NULL) {
        fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(file, "<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\">\n");
        fprintf(file, "\t<Nodes>\n");
        generateDGMLforNode(file, tree);
        fprintf(file, "\t</Nodes>\n");
        fprintf(file, "\t<Links>\n");
        generateDGMLforLinks(file, tree);
        fprintf(file, "\t</Links>\n");
        fprintf(file, "</DirectedGraph>\n");
        fclose(file);
    }
}


myTreeNode* parseText(sourceFileText *source)
{
    myTreeNode* resultTree = NULL;

    pANTLR3_INPUT_STREAM input = antlr3StringStreamNew((pANTLR3_UINT8)source->text, ANTLR3_ENC_UTF8, source->length, (pANTLR3_UINT8)source->fileName);
    if (input == NULL)
    {
        fprintf(stderr, "Unable to parse file %s\n", source->fileName);
    }
    else
    {
        pGrammarCLexer lxr = GrammarCLexerNew(input);	    // CLexerNew is generated by ANTLR
        if (lxr == NULL)
        {
            fprintf(stderr, "Unable to create the lexer due to malloc() failure1\n");
        }
        else
        {
            pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
            if (tstream == NULL)
            {
                fprintf(stderr, "Out of memory trying to allocate token stream\n");
                return NULL;
            }
            else
            {
                pGrammarCParser psr = GrammarCParserNew(tstream);  // CParserNew is generated by ANTLR3
                if (psr == NULL)
                {
                    fprintf(stderr, "Out of memory trying to allocate parser\n");
                    return NULL;
                }
                else
                {
                    GrammarCParser_program_return SimpleCAST = psr->program(psr);;

                    if (psr->pParser->rec->state->errorCount > 0)
                    {
                        fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->state->errorCount);
                        return NULL;
                    }
                    else
                    {
                        // traverseTree(SimpleCAST.tree, 0);

                        resultTree = translateAntlrTree(SimpleCAST.tree);
                    }

                   /* if (SimpleCAST.tree)
                    {
                        SimpleCAST.tree->free(SimpleCAST.tree);
                    }*/

                    psr->free(psr);
                }
            
                tstream->free(tstream);
            }

            lxr->free(lxr);
        }

        input->close(input);
    }

    return resultTree;
}
