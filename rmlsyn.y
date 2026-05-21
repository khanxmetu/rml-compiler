%language "c++"
%define api.parser.class {MyParserBase}
   
%{
using namespace std;
#include <iostream>
#include <fstream>
%}
%code requires
{
#include <vector>
#include "rmlsup.h"
};

%code 
{
   #include "MyParser.h"
   #define yylex(x) driver->lex(x)
}

%define api.value.type union
%nterm <RMLSelectNode *>selectStatement
%nterm <RMLSelectNode *>selectOp
%nterm <RMLColumnDef *>columnDef 
%nterm <RMLEvalExpNode *>expression
%nterm <RMLEvalExpNode *>commaOperand
%nterm <RMLEvalExpNode *>ternaryComponent
%nterm <RMLEvalExpNode *>term
%nterm <RMLEvalExpNode *>booleanOperand
%nterm <RMLEvalExpNode *>comparisonOperand
%nterm <RMLEvalExpNode *>factor
%nterm <RMLEvalExpNode *>factorOperand
%nterm <RMLEvalExpNode *>unaryOperand
%nterm <RMLEvalExpNode *>column
%nterm <RMLEvalExpNode *>memberSpec
%nterm <RMLEvalExpNode *>subExpression
%nterm <RMLEvalExpNode *>productSpace
%nterm <RMLEvalSpaceNode *>space
%nterm <RMLColumnList *>columnList
%nterm <RMLEvalExpNode *>condition
%nterm <int>booleanOperator
%nterm <int>comparisonOperator
%nterm <int>termOperator
%nterm <int>factorOperator
%nterm <RMLEvalExpNode *>alias
%nterm <int>unaryOperator
%nterm <RMLEvalExpNode *>funcRef
%nterm <RMLEvalExpNode *>actualParameters

%parse-param {MyParser *driver} 
%start rml

%token PLUS
%token MINUS
%token MUL
%token DIV
%token LP
%token RP
%token LT
%token LTE
%token GT
%token GTE
%token EQ
%token NEQ
%token BAND
%token BOR
%token AS
%token <string *>ID
%token <string *>STR
%token <double> NUM
%token COMMA
%token DOT
%token NOT
%token SEMICOLON
%token COLON
%token QMARK
%token WAIT
%token THEN
%token TRUE
%token FALSE
%token SELECT
%token FROM
%token WHERE

%%

rml   : selectStatement                                     
      | rml selectStatement                                 ;

selectStatement
      : selectOp SEMICOLON                                  {$$=driver->processSelectStat($1);};

selectOp
      : SELECT columnList FROM productSpace WHERE condition {$$=driver->processSelectOp($2, $4, $6);}; 
      
columnList
      : columnList COMMA columnDef                          {$$=driver->appendToColumnList($1, $3);}
      | columnDef                                           {$$=driver->createColumnList($1);};
      
columnDef
      : expression AS ID                                    {$$=driver->createColumnDef($1, $3);};
      
productSpace
      : productSpace COMMA space                            {$$=driver->appendToProductSpace($1, $3);}
      | space                                               {$$=$1;};
      
space : ID alias                                            {$$=driver->processExternalSpace($1, $2);}
      | LP selectOp RP alias                                {$$=driver->processCalculatedSpace($2, $4);};  
      
alias : ID                                                  {$$=driver->processAlias($1);}
      |                                                     {$$=nullptr;};
      
condition
      : expression                                          {$$=$1;};
        
expression
	  : commaOperand                          {$$=$1;}
      | expression COMMA commaOperand         {$$=driver->processBILRNode($1, OP_COMMA, $3);};
		
commaOperand
		: booleanOperand ternaryComponent       {$$=driver->processCondition($1,$2);};
		
ternaryComponent
		: QMARK booleanOperand COLON booleanOperand       {$$=driver->processBILRNode($2, OP_ALT, $4);}
		| {$$=nullptr;};
		
booleanOperand
      : comparisonOperand                                {$$=$1;}
      | booleanOperand booleanOperator comparisonOperand {$$=driver->processBILRNode($1, $2, $3);};
      
booleanOperator
      : BAND                                           {$$=OP_BAND;}
      | BOR                                            {$$=OP_BOR;};
      
comparisonOperand
      : term                                           {$$=$1;}
      | comparisonOperand comparisonOperator term      {$$=driver->processBILRNode($1, $2, $3);};
      
comparisonOperator
      : EQ                                             {$$=OP_EQ;}
      | NEQ                                            {$$=OP_NEQ;}
      | LT                                             {$$=OP_LT;}
      | LTE                                            {$$=OP_LTE;}
      | GT                                             {$$=OP_GT;}
      | GTE                                            {$$=OP_GTE;};
      
term  : factor                                         {$$=$1;}
      | term termOperator factor                       {$$=driver->processBILRNode($1, $2, $3);};
      
termOperator
      : PLUS                                           {$$=OP_ADD;}
      | MINUS                                          {$$=OP_SUB;};
      
factor
      : factorOperand                                  {$$=$1;}
      | factor factorOperator factorOperand            {$$=driver->processBILRNode($1, $2, $3);};
      
factorOperator
		: MUL                                            {$$=OP_MUL;}
		| DIV                                            {$$=OP_DIV;};
		
factorOperand
		: unaryOperator unaryOperand               {$$=driver->processBinaryOperand($1,$2);}
		| unaryOperand                             {$$=$1;};
		
unaryOperator
		: MINUS                                    {$$=OP_MINUS;}
		| NOT                                      {$$=OP_NOT;};
		
unaryOperand
      : NUM                                      {$$=driver->processNumericConstant($1);}
	   | TRUE                                     {$$=driver->processBooleanConstant(true);}
	   | FALSE                                    {$$=driver->processBooleanConstant(false);}
	   | STR                                      {$$=driver->processStringConstant($1);}	
	   | column                                   {$$=$1;}
	   | funcRef LP actualParameters RP           {$$=driver->processCallOrSubExp($1,$3);}
	   | LP subExpression RP                      {$$=driver->processCallOrSubExp(nullptr,$2);};
	   
funcRef: 
      ID                                         {$$=driver->processFuncRef($1);};
		
actualParameters
      : expression                               {$$=$1;}
      |                                          {$$=nullptr;};
		
subExpression
      : selectOp                                 {$$=$1;}
      | expression                               {$$=$1;};	
      	
column: ID memberSpec                            {$$=driver->processColumn($1, $2);};

memberSpec
      : DOT ID                                   {$$=driver->processMember($2);}
      |                                          {$$=nullptr;};
   	
%%