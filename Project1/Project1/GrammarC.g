grammar GrammarC;
options 
{
    output			= AST;
    language		= C;
	ASTLabelType	= pANTLR3_BASE_TREE;
    backtrack       = true;
    memoize         = true;
}

tokens 
{ 
    ARG_DEF;
    FUNC_HDR;
    FUNC_DECL;
    FUNC_DEF;
    BLOCK;
    IF_STMT;
    DO_STMT;
    WHILE_STMT;
    BREAK_STMT;
    VAR_STMT;
    AST_EXPR_EQUAL;
    AST_EXPR_LESS;
    AST_EXPR_GREATER;
    AST_EXPR_DIV;
    AST_EXPR_MUL;
    AST_EXPR_SUB;
    AST_EXPR_SUM;
    AST_EXPR_INV;
    AST_EXPR_NEG;
    AST_EXPR_NOT;
    AST_KW_BOOL;
    AST_KW_BYTE;
    AST_KW_INT;
    AST_KW_UINT;
    AST_KW_LONG;
    AST_KW_ULONG;
    AST_KW_CHAR;
    AST_KW_STRING;
    AST_KW_VOID;
    AST_TYPE_REF;
    AST_TYPE_REF;
    AST_ID;
    AST_ARR_DIM_SEPARATOR;
    AST_ARR_DIM_SPEC ;
    AST_BLOCK;
}



StringLiteral
	: '"' DoubleStringCharacter* '"'
	| '\'' SingleStringCharacter* '\''
	;
	
fragment DoubleStringCharacter
	: ~('"' | '\\')	
	| '\\' EscapeSequence
	;

fragment SingleStringCharacter
	: ~('\'' | '\\')	
	| '\\' EscapeSequence
	;
	
fragment EscapeSequence
	: SingleEscapeCharacter
	| NonEscapeCharacter
	;

fragment NonEscapeCharacter
	: ~(EscapeCharacter  )
	;

fragment SingleEscapeCharacter
	: '\'' | '"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v'
	;

fragment EscapeCharacter
	: SingleEscapeCharacter
    ;




identifier: s=ID -> ^(AST_ID $s); 

STR :   '"\"[^\"\\]*(?:\\.[^\"\\]*)*\"' ;
CHAR:   '[''^'']' ;
BITSCONST :   ('0') ('b'|'B') ('0'|'1')+ ;
BOOL:   ('true'|'false') ;
ID  :   ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')* ;
INT :	('0'..'9')+ ;
HEXCONST:   ('0') ('x'|'X') ('0'..'9'|'A'..'F'|'a'..'f')+ ;





EQ   : '==' ;
ASSIGN : '=' ;
PLUS : '+' ;
COMMA: ',';

WS  :   (   ' '
        |   '\t'
        |   '\r'
        |   '\n'
        )+
        { $channel=HIDDEN; }
    ;    

program
    :   funcDef*
    ;
    
type: typeAtom arrayTypeSpec* -> ^(AST_TYPE_REF typeAtom arrayTypeSpec*);

typeAtom
    :   'int'   -> ^(AST_KW_INT)     
    |   'char'  -> ^(AST_KW_CHAR)
    |   'void'  -> ^(AST_KW_VOID)
    |   'byte'  -> ^(AST_KW_BYTE)
    |   'bool'  -> ^(AST_KW_BOOL)
    |   'uint'  -> ^(AST_KW_UINT)
    |   'long'  -> ^(AST_KW_LONG)
    |   'ulong'  -> ^(AST_KW_ULONG)
    |   'string' -> ^(AST_KW_STRING)
    |   identifier
    ;

arrayTypeSpec: '(' arrayTypeSpecStep* ')' -> ^(AST_ARR_DIM_SPEC arrayTypeSpecStep*);
arrayTypeSpecStep: COMMA -> ^(AST_ARR_DIM_SEPARATOR);

block
    :   statement* -> ^(AST_BLOCK statement*)
    ;

statement
    : iterationStatement
    | expr ';'!
    | selectionStatement
    | jumpStatement
    | varStatement 
    ;

varStatement
   :   'dim' e=identifier (COMMA r=identifier)* 'as' t=type
            -> ^(VAR_STMT $e $r $t)
   ;

selectionStatement
    :   'if' e=expr 'then' s1=statement ('else' s2=statement)? 'end' 'if'
            -> ^(IF_STMT $e $s1 $s2)
    ;

iterationStatement
    :   'while' e=expr s=statement* 'wend'
            -> ^(WHILE_STMT $e $s)
    |   'do' s=statement* 'loop' k=doLoopKind e=expr
            -> ^(DO_STMT $s $k $e)
    ;
doLoopKind: 'while'|'until';

jumpStatement
    :   'break' -> BREAK_STMT
    ;

expr:   callOrIndexerExpr
    |   aexpr
    |   condExpr
    |   sexpr
    |   mexpr
    |   unary
    |   atom;

literal: STR|CHAR|BITSCONST|BOOL|INT|HEXCONST;

atom:   bracesExpr
    |   literal
    |   identifier;

unary: not|neg|inv;
neg: '-' (atom|unary)  -> AST_EXPR_NEG; 
not: '!' (atom|unary)  -> AST_EXPR_NOT; 
inv: '~' (atom|unary)  -> AST_EXPR_INV; 


bracesExpr
    :  '(' expr^ ')'
    ;

callOrIndexerExpr
    :   atom operandsSpec+
    ;
operandsSpec: '(' expr (COMMA expr)* ')';

aexpr: aarg (ASSIGN^ (aarg|aexpr));
aarg: condExpr|sexpr|mexpr|unary|atom;

condExpr: condArg (comparisonOperator^ (condArg|condExpr));
condArg: sexpr|mexpr|unary|atom;

comparisonOperator
    : EQ -> AST_EXPR_EQUAL
    | '<' -> AST_EXPR_LESS
    | '>' -> AST_EXPR_GREATER
    ;

sexpr:   sarg ( sop^ (sarg|sexpr) );
sarg: mexpr|unary|atom;

mexpr:   marg ( mop^ (marg|mexpr) );
marg: unary|atom;

sop:  '+' -> AST_EXPR_SUM
	| '-' -> AST_EXPR_SUB
    ;
mop: '*' -> AST_EXPR_MUL
	| '\\' -> AST_EXPR_DIV
	;


formalParameter
    :   identifier ('as' type)?
				-> ^(ARG_DEF type identifier)
    ;

funcSignature
    :   identifier '(' ( formalParameter ( COMMA formalParameter )* )? ')' ('as' type)?
				-> ^(FUNC_HDR type identifier formalParameter*)
    ;
    
funcDef: 'function' funcSignature block 'end' 'function' -> ^(FUNC_DECL funcSignature block );


