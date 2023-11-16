#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
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

int main(int argc, char* argv[])
{
    char* fName;

    if (argc < 2 || argv[1] == NULL)
    {
        fName = "./input"; // Note in VS2005 debug, working directory must be configured
    }
    else
    {
        fName = argv[1];
    }
    
    sourceFileText sourceFile;
    if (tryReadFile(fName, &sourceFile))
    {
        myTreeNode* ast = parseText(&sourceFile);

        //buildCFG(SimpleCAST.tree, graph, 0);

        formatAstAsDgmlToFile("tree.dgml", ast);
    }
    


}