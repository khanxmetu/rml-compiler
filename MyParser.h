#ifndef MYPARSER_H_
#define MYPARSER_H_

#include <unistd.h>
#include <string.h>
// #include <malloc.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include "rmlsup.h"

namespace yy
{
   class MyParserBase;
};

class MyFlexLexer;

class MyParser
{
   yy::MyParserBase *base=nullptr;
   MyFlexLexer      *lexer=nullptr;

   // Variable to integrate token semantics
   yy::MyParserBase::semantic_type *lval=nullptr;

   int              parseErrorLine=-1;

   string *makeString(const char *rawStr);
   int hexDigit(char c);
public:
   int      optimization;
   RMLEval *rmlEval=nullptr;

   MyParser(int pOptimization);
   ~MyParser();

   void compile(yy::MyParserBase *pBase, ifstream *is, RMLEval *rmlEval);
   int lex(yy::MyParserBase::value_type *lval);

   int getId();
   int getStr();
   int getNumber();

   void setParseErrorLine();
   int getParseErrorLine();
   vector<RMLSelectNode *>	stats;
   int   selectOpCount=0,
         spaceCount=0,
         aliasCount=0,
		   numericConstantCount=0,
		   stringConstantCount=0,
		   variableReferenceCount=0,
		   fqVariableReferenceCount=0,
		   funcCallCount=0;
   int   nextSelectStatIndex=0;
   // Parser actions
   RMLEvalExpNode *processBILRNode(RMLEvalExpNode *baseNode, int opCode, RMLEvalExpNode *newOperand);
   RMLEvalExpNode *processBIRLNode(RMLEvalExpNode *baseNode, int opCode, RMLEvalExpNode *newOperand);
   //RMLEvalExpNode *processAssignmentSide(RMLEvalExpNode *conditionNode, RMLEvalExpNode *alternatives);
   RMLEvalExpNode *processBinaryOperand(int pOpCode, RMLEvalExpNode *operand);
   RMLEvalExpNode *processCallOrSubExp(RMLEvalExpNode *funcRef, RMLEvalExpNode *subExp);
   RMLEvalExpNode *processFuncRef(string *funcName);
   RMLEvalExpNode *processCondition(RMLEvalExpNode *booleanOperand, RMLEvalExpNode *alternatives);

   RMLColumnDef *createColumnDef(RMLEvalExpNode *expNode, string *id);
   RMLColumnList *createColumnList(RMLColumnDef *pColumnDef);
   RMLColumnList *appendToColumnList(RMLColumnList *columnList, RMLColumnDef *pColumnDef);

   /*
   RMLEvalExpNode *createPostfixNode(int pOpCode, RMLEvalExpNode *operand);
   RMLEvalExpNode *processUnaryOperand(RMLEvalExpNode *baseOperand, RMLEvalExpNode *postfixOperand);
	*/

   RMLSelectNode *processSelectStat(RMLSelectNode *selectOp);
   RMLSelectNode *processSelectOp(RMLColumnList *columnList, RMLEvalExpNode *productSpace, RMLEvalExpNode *condition);

   RMLEvalSpaceNode *processExternalSpace(string *externalSpaceNode, RMLEvalExpNode *alias);
   RMLEvalSpaceNode *processCalculatedSpace(RMLSelectNode *calculatedSpaceNode, RMLEvalExpNode *alias);

   RMLEvalExpNode *appendToProductSpace(RMLEvalExpNode *productSpace, RMLEvalExpNode *space);
   RMLEvalExpNode *processAlias(string *id);

   RMLEvalExpNode *processNumericConstant(double value);
   RMLEvalExpNode *processBooleanConstant(bool value);
   RMLEvalExpNode *processStringConstant(string *str);
   RMLEvalExpNode *processColumn(string *spaceId, RMLEvalExpNode *member);
   RMLEvalExpNode *processMember(string *id);
};

#include "x64codegen.h"

#endif /* MYPARSER_H_ */
