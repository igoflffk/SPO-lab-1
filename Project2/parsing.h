#pragma once

typedef struct {
    char* fileName;
    char* text;
    int length;
} sourceFileText;


typedef struct myTreeNode {
    int id;
    int kind;
    char* text;
    int childrenCount;
    struct myTreeNode** children;
} myTreeNode;


myTreeNode* parseText(sourceFileText* source);

void formatAstAsDgmlToFile(const char* outputFile, myTreeNode* tree);


