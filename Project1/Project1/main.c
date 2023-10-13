// Example of a grammar for a tree parser.
// Adapted from Java equivalent example, by Terence Parr
// Author: Jim Idle - April 2007
// Permission is granted to use this example code in any way you want, so long as
// all the original authors are cited.
//

// set ts=4,sw=4
// Tab size is 4 chars, indent is 4 chars

// Notes: Although all the examples provided are configured to be built
//        by Visual Studio 2005, based on the custom build rules
//        provided in $(ANTLRSRC)/code/antlr/main/runtime/C/vs2005/rulefiles/antlr3.rules
//        there is no reason that this MUST be the case. Provided that you know how
//        to run the antlr tool, then just compile the resulting .c files and this
//	  file together, using say gcc or whatever: gcc *.c -I. -o XXX
//	  The C code is generic and will compile and run on all platforms (please
//        report any warnings or errors to the antlr-interest newsgroup (see www.antlr.org)
//        so that they may be corrected for any platform that I have not specifically tested.
//
//	  The project settings such as additional library paths and include paths have been set
//        relative to the place where this source code sits on the ANTLR perforce system. You
//        may well need to change the settings to locate the includes and the lib files. UNIX
//        people need -L path/to/antlr/libs -lantlr3c (release mode) or -lantlr3cd (debug)
//
//        Jim Idle (jimi cut-this at idle ws)
//

// You may adopt your own practices by all means, but in general it is best
// to create a single include for your project, that will include the ANTLR3 C
// runtime header files, the generated header files (all of which are safe to include
// multiple times) and your own project related header files. Use <> to include and
// -I on the compile line (which vs2005 now handles, where vs2003 did not).
//
#include    "GrammarCLexer.h"
#include    "GrammarCParser.h"
//#include    "stdio.h"
// #include    "GrammarCWalker.h"


typedef struct myTreeNode {
    int id;
    int kind;
    char *text;
    int childrenCount;
    struct myTreeNode** children;
} myTreeNode;

typedef struct antlrTreeTranslationState {
    int idCounter;
} antlrTreeTranslationState;

myTreeNode* translateAntlrTreeImpl(pANTLR3_BASE_TREE node, antlrTreeTranslationState *state)
{
    int childrenCount = node->getChildCount(node);

    myTreeNode* curr = (myTreeNode*)malloc(sizeof(myTreeNode));
    curr->id = state->idCounter++;
    curr->kind = node->getType(node);
    curr->text = node->toString(node)->chars;
    curr->children = (myTreeNode**)malloc(sizeof(myTreeNode*) * childrenCount);
    curr->childrenCount = childrenCount;

    printf("Line: %d  ", node->getToken(node)->line);
    printf("Type: %d  ", node->getType(node) );
    printf("Id:   %s\n ", node->toString(node)->chars);

    for (int i = 0; i < childrenCount; i++) {
        curr->children[i] = translateAntlrTreeImpl(node->getChild(node, i), state);
    }

    return curr;
}

myTreeNode* translateAntlrTree(pANTLR3_BASE_TREE node)
{
    antlrTreeTranslationState state = { 0 };
    return translateAntlrTreeImpl(node, &state);
}



void traverseTree(pANTLR3_BASE_TREE node, int depth)
{
    for (int i = 0; i < depth; i++) {
        printf("+-->");
    }
    // node->getToken(node)->line

    printf("%d:%s\n", node->getType(node), node->toString(node)->chars);

    for (int i = 0; i < node->getChildCount(node); i++) {
        traverseTree(node->getChild(node, i), depth + 1);
    }
}


void generateDGMLforNode(FILE* file, myTreeNode* node)
{
    fprintf(file, "\t\t<Node Id = \" %d \" Label = \" %s \"/>\n", node->id, node->text);
    for (int i = 0; i < node->childrenCount; i++) {
        generateDGMLforNode(file, node->children[i]);
    }
}

void generateDGMLforLinks(FILE* file, myTreeNode* node)
{
    for (int i = 0; i < node->childrenCount; i++) {
        fprintf(file, "\t\t<Link Source=\" %d \" Target=\" %d \" />\n", node->id, node->children[i]->id);
        generateDGMLforLinks(file, node->children[i]);
    }
}


void generateDGMLfile( const char* outputFile, myTreeNode* tree) {

    FILE* file = fopen(outputFile, "a");

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




// Main entry point for this example
//
int ANTLR3_CDECL
main(int argc, char* argv[])
{
    // Now we declare the ANTLR related local variables we need.
    // Note that unless you are convinced you will never need thread safe
    // versions for your project, then you should always create such things
    // as instance variables for each invocation.
    // -------------------

    // Name of the input file. Note that we always use the abstract type pANTLR3_UINT8
    // for ASCII/8 bit strings - the runtime library guarantees that this will be
    // good on all platforms. This is a general rule - always use the ANTLR3 supplied
    // typedefs for pointers/types/etc.
    //
    pANTLR3_UINT8	    fName;

    // The ANTLR3 character input stream, which abstracts the input source such that
    // it is easy to provide input from different sources such as files, or 
    // memory strings.
    //
    // For an ASCII/latin-1 memory string use:
    //	    input = antlr3NewAsciiStringInPlaceStream (stringtouse, (ANTLR3_UINT64) length, NULL);
    //
    // For a UCS2 (16 bit) memory string use:
    //	    input = antlr3NewUCS2StringInPlaceStream (stringtouse, (ANTLR3_UINT64) length, NULL);
    //
    // For input from a file, see code below
    //
    // Note that this is essentially a pointer to a structure containing pointers to functions.
    // You can create your own input stream type (copy one of the existing ones) and override any
    // individual function by installing your own pointer after you have created the standard 
    // version.
    //
    pANTLR3_INPUT_STREAM	    input;

    // The lexer is of course generated by ANTLR, and so the lexer type is not upper case.
    // The lexer is supplied with a pANTLR3_INPUT_STREAM from whence it consumes its
    // input and generates a token stream as output.
    //
    pGrammarCLexer			    lxr;

    // The token stream is produced by the ANTLR3 generated lexer. Again it is a structure based
    // API/Object, which you can customise and override methods of as you wish. a Token stream is
    // supplied to the generated parser, and you can write your own token stream and pass this in
    // if you wish.
    //
    pANTLR3_COMMON_TOKEN_STREAM	    tstream;

    // The Lang parser is also generated by ANTLR and accepts a token stream as explained
    // above. The token stream can be any source in fact, so long as it implements the 
    // ANTLR3_TOKEN_SOURCE interface. In this case the parser does not return anything
    // but it can of course specify any kind of return type from the rule you invoke
    // when calling it.
    //
    pGrammarCParser			    psr;

    // The parser produces an AST, which is returned as a member of the return type of
    // the starting rule (any rule can start first of course). This is a generated type
    // based upon the rule we start with.
    //
    GrammarCParser_program_return	    SimpleCAST;


    // The tree nodes are managed by a tree adaptor, which doles
    // out the nodes upon request. You can make your own tree types and adaptors
    // and override the built in versions. See runtime source for details and
    // eventually the wiki entry for the C target.
    //
    pANTLR3_COMMON_TREE_NODE_STREAM	nodes;

    // Finally, when the parser runs, it will produce an AST that can be traversed by the 
    // the tree parser: c.f. SimpleCWalker.g3t
    //
    //pGrammarCWalker		    treePsr;
    


    // Create the input stream based upon the argument supplied to us on the command line
    // for this example, the input will always default to ./input if there is no explicit
    // argument.
    //
    if (argc < 2 || argv[1] == NULL)
    {
        fName = (pANTLR3_UINT8)"./input"; // Note in VS2005 debug, working directory must be configured
    }
    else
    {
        fName = (pANTLR3_UINT8)argv[1];
    }

    // Create the input stream using the supplied file name
    // (Use antlr3AsciiFileStreamNew for UCS2/16bit input).
    //
    input = antlr3FileStreamNew(fName, ANTLR3_ENC_UTF8);

    // The input will be created successfully, providing that there is enough
    // memory and the file exists etc
    //
    if (input == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", (char*)fName);
        exit(1);
    }

    // Our input stream is now open and all set to go, so we can create a new instance of our
    // lexer and set the lexer input to our input stream:
    //  (file | memory | ?) --> inputstream -> lexer --> tokenstream --> parser ( --> treeparser )?
    //
    lxr = GrammarCLexerNew(input);	    // CLexerNew is generated by ANTLR

    // Need to check for errors
    //
    if (lxr == NULL)
    {
        fprintf(stderr, "Unable to create the lexer due to malloc() failure1\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // Our lexer is in place, so we can create the token stream from it
    // NB: Nothing happens yet other than the file has been read. We are just 
    // connecting all these things together and they will be invoked when we
    // call the parser rule. ANTLR3_SIZE_HINT can be left at the default usually
    // unless you have a very large token stream/input. Each generated lexer
    // provides a token source interface, which is the second argument to the
    // token stream creator.
    // Note tha even if you implement your own token structure, it will always
    // contain a standard common token within it and this is the pointer that
    // you pass around to everything else. A common token as a pointer within
    // it that should point to your own outer token structure.
    //
    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));

    if (tstream == NULL)
    {
        fprintf(stderr, "Out of memory trying to allocate token stream\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // Finally, now that we have our lexer constructed, we can create the parser
    //
    psr = GrammarCParserNew(tstream);  // CParserNew is generated by ANTLR3

    if (tstream == NULL)
    {
        fprintf(stderr, "Out of memory trying to allocate parser\n");
        exit(ANTLR3_ERR_NOMEM);
    }

    // We are all ready to go. Though that looked complicated at first glance,
    // I am sure, you will see that in fact most of the code above is dealing
    // with errors and there isn;t really that much to do (isn;t this always the
    // case in C? ;-).
    //
    // So, we now invoke the parser. All elements of ANTLR3 generated C components
    // as well as the ANTLR C runtime library itself are pseudo objects. This means
    // that they are represented as pointers to structures, which contain any
    // instance data they need, and a set of pointers to other interfaces or
    // 'methods'. Note that in general, these few pointers we have created here are
    // the only things you will ever explicitly free() as everything else is created
    // via factories, that allocates memory efficiently and free() everything they use
    // automatically when you close the parser/lexer/etc.
    //
    // Note that this means only that the methods are always called via the object
    // pointer and the first argument to any method, is a pointer to the structure itself.
    // It also has the side advantage, if you are using an IDE such as VS2005 that can do it
    // that when you type ->, you will see a list of all the methods the object supports.
    //
    SimpleCAST = psr->program(psr);

    
    // If the parser ran correctly, we will have a tree to parse. In general I recommend
    // keeping your own flags as part of the error trapping, but here is how you can
    // work out if there were errors if you are using the generic error messages
    //
    if (psr->pParser->rec->state->errorCount > 0)
    {
        fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->state->errorCount);

    }
    else
    {
        
        traverseTree(SimpleCAST.tree, 0);

        myTreeNode* t = translateAntlrTree(SimpleCAST.tree);

        generateDGMLfile("tree.dgml", t);
        
        printf("s");

        // Tree parsers are given a common tree node stream (or your override)
        //
        // treePsr = GrmmarCWalkerNew(nodes);

        // treePsr->program(treePsr);
        // nodes->free(nodes); nodes = NULL;
        // treePsr->free(treePsr);	    treePsr = NULL;
    }

    // We did not return anything from this parser rule, so we can finish. It only remains
    // to close down our open objects, in the reverse order we created them
    //

    psr->free(psr);	    psr = NULL;
    tstream->free(tstream);	    tstream = NULL;
    lxr->free(lxr);	    lxr = NULL;
    input->close(input);	    input = NULL;

    return 0;
}