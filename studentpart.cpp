using namespace std;
// #include <bits/stdc++.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include "rmlsup.h"

#include "rmlsyn.tab.hh"
#include "MyParser.h"
#include "FlexLexer.h"
#include "MyFlexLexer.h"

// Parser actions

RMLColumnDef *MyParser::createColumnDef(RMLEvalExpNode *expNode, string *id)
{
   return new RMLColumnDef(expNode, id);
}

RMLColumnList *MyParser::createColumnList(RMLColumnDef *columnDef)
{
   return new RMLColumnList(columnDef);
}

RMLColumnList *MyParser::appendToColumnList(RMLColumnList *columnList, RMLColumnDef *columnDef)
{
   columnList->addColumn(columnDef);
   return columnList;
}

RMLEvalExpNode *MyParser::processBILRNode(RMLEvalExpNode *pLeft, int opCode, RMLEvalExpNode *pRight)
{
   return new RMLEvalExpNode(lexer->lineno(), pLeft, opCode, pRight);
}

RMLEvalExpNode *MyParser::processBIRLNode(RMLEvalExpNode *pLeft, int opCode, RMLEvalExpNode *pRight)
{
   RMLEvalExpNode *cursor=pLeft,
                 *parent=nullptr;

   while (cursor->right!=nullptr)
   {
      parent=cursor;
      cursor=cursor->right;
   }

   if (parent==nullptr)
      return new RMLEvalExpNode(lexer->lineno(), pLeft, opCode, pRight);

   parent->right=new RMLEvalExpNode(lexer->lineno(), cursor, opCode, pRight);
   parent->right->parent=parent;

   return pLeft;
}

RMLEvalExpNode *MyParser::processBinaryOperand(int pOpCode, RMLEvalExpNode *operand)
{
   return new RMLEvalExpNode(lexer->lineno(), operand, pOpCode);
}

RMLEvalExpNode *MyParser::processCallOrSubExp(RMLEvalExpNode *funcRef, RMLEvalExpNode *subExp)
{
   if (funcRef!=nullptr)
      return new RMLEvalExpNode(lexer->lineno(), funcRef, OP_CALL, subExp);
   return subExp;
}

RMLEvalSpaceNode *MyParser::processExternalSpace(string *externalSpaceName, RMLEvalExpNode *alias)
{
#ifdef PROJECT02
   spaceCount++;
   return nullptr;
#else
   RMLEvalSpaceNode *node=new RMLEvalExternalSpaceNode(lexer->lineno(), externalSpaceName, alias);

   return node;
#endif
}

RMLEvalSpaceNode *MyParser::processCalculatedSpace(RMLSelectNode *calculatedSpaceNode, RMLEvalExpNode *alias)
{

#ifdef PROJECT02
   spaceCount++;
   return nullptr;
#else
   RMLEvalSpaceNode *node=new RMLEvalCalculatedSpaceNode(lexer->lineno(), calculatedSpaceNode, alias);
   return node;
#endif
}

RMLSelectNode *MyParser::processSelectOp(RMLColumnList *columnList, RMLEvalExpNode *productSpace, RMLEvalExpNode *condition)
{
   RMLSelectNode *node=new RMLSelectNode(lexer->lineno(), columnList, productSpace, OP_SELECT, condition);
   selectOpCount++;

   return node;
}

RMLSelectNode *MyParser::processSelectStat(RMLSelectNode *selectOp)
{
   stats.push_back(selectOp);

   return selectOp;
}

RMLEvalExpNode *MyParser::processCondition(RMLEvalExpNode *booleanOperand, RMLEvalExpNode *alternatives)
{
   if (alternatives==nullptr)
     return booleanOperand;

   return new RMLEvalExpNode(lexer->lineno(), booleanOperand, OP_COND, alternatives);
}

RMLEvalExpNode *MyParser::appendToProductSpace(RMLEvalExpNode *productSpace, RMLEvalExpNode *space)
{
   return new RMLEvalExpNode(lexer->lineno(), productSpace, OP_CROSSP, space);
}

RMLEvalExpNode *MyParser::processNumericConstant(double value)
{
   numericConstantCount++;
   return new RMLEvalExpNode(lexer->lineno(), value, CONST);
}

RMLEvalExpNode *MyParser::processAlias(string *id)
{
   aliasCount++;
   return new RMLEvalExpNode(lexer->lineno(), id, INSID);
}

RMLEvalExpNode *MyParser::processStringConstant(string *value)
{
   stringConstantCount++;
   return new RMLEvalExpNode(lexer->lineno(), value, CONST);
}

RMLEvalExpNode *MyParser::processColumn(string *spaceId, RMLEvalExpNode *member)
{
   variableReferenceCount++;
   return new RMLEvalExpNode
         (
               lexer->lineno(),
               member!=nullptr?spaceId:nullptr,
               member!=nullptr?member:new RMLEvalExpNode(lexer->lineno(), spaceId, INSMEM),
               SYMREF
         );
}

RMLEvalExpNode *MyParser::processFuncRef(string *funcName)
{
   funcCallCount++;
   return new RMLEvalExpNode(lexer->lineno(), funcName, INSFREF);
}

RMLEvalExpNode *MyParser::processMember(string *memberId)
{
   fqVariableReferenceCount++;
   return new RMLEvalExpNode(lexer->lineno(), memberId, INSMEM);
}

RMLEvalExpNode *MyParser::processBooleanConstant(bool value)
{
   return new RMLEvalExpNode(lexer->lineno(), value, CONST);
}

// DGEvalMsgSet methods
void RMLMsgSet::writeAsJSON(ostream *os)
{

   auto                 c=messages.begin();
   size_t               cnt=messages.size();

   (*os)<<"[";

   for (size_t i=0;i<cnt;i++, c++)
   {
      if (i>0)
         (*os)<<", ";

      if ((*c)->lineNumber==INT_MAX || (*c)->lineNumber==INT_MIN)
         (*os)<<"\"["+RMLEval::severityStr[(int)(*c)->s]+"]: "+(*c)->msg+"\"";
      else
         (*os)<<"\"Line number "+to_string((*c)->lineNumber)+" ["+RMLEval::severityStr[(int)(*c)->s]+"]: "+(*c)->msg+"\"";
   }
   (*os)<<"]";
}

void RMLMsgSet::writeToConsole()
{
   auto                 c=messages.begin();
   size_t               cnt=messages.size();

   for (size_t i=0;i<cnt;i++, c++)
   {
      if ((*c)->lineNumber==INT_MAX || (*c)->lineNumber==INT_MIN)
         cout<<"["+RMLEval::severityStr[(int)(*c)->s]+"]: "+(*c)->msg;
      else
         cout<<"Line number "+to_string((*c)->lineNumber)+" ["+RMLEval::severityStr[(int)(*c)->s]+"]: "+(*c)->msg;

      cout << endl;
   }
}

void RMLEval::scanCheckExtractSymbols()
{
   int sc=stats->size();
   for (int i=0;i<sc;i++)
   {
      RMLSelectNode *statNode=(*stats)[i];

#if defined(PROJECT04)
      statNode->selectStatRuntimeIndex=(int)ss.size();
      ss.push_back(RMLSpaceRuntime((*stats)[i]));
#endif
      scanSelectNodeForSymbols(nullptr, statNode);
   }
}

void RMLEval::populateSelectNodeSymbols(RMLSelectNode *node, RMLEvalSpaceNode *spaceNode)
{
   string         *spaceName=spaceNode->getSpaceName(),
                  *spaceAlias=spaceNode->getSpaceAlias();

   if (spaceName!=nullptr && findLibFunction(*spaceName)!=nullptr)
      messageSet.appendMessage(node->lineNumber, "Space name "+*spaceName+" is not allowed as it is a library function.", RMLEvalMsgSeverity::Error);

   if (spaceAlias!=nullptr && findLibFunction(*spaceAlias)!=nullptr)
      messageSet.appendMessage(node->lineNumber, "Alias name "+*spaceAlias+" is not allowed as it is a library function.", RMLEvalMsgSeverity::Error);

   RMLSymbolTable *symbolTable=&node->symbolTable;
   int cc=spaceNode->columnCount();

   for (int i=0;i<cc;i++)
   {
      RMLColumnSpec   *columnSpec=spaceNode->columnAt(i);
      RMLSymbolColumn *symbolColumn=symbolTable->addSymbol(spaceName, spaceAlias, columnSpec->name, columnSpec->type);
      string          *columnName=columnSpec->name;

      if (findLibFunction(*columnName)!=nullptr)
         messageSet.appendMessage(node->lineNumber, "Column name "+*columnName+" is not allowed as it is a library function.", RMLEvalMsgSeverity::Error);

      if (symbolColumn==nullptr)
         messageSet.appendMessage(node->lineNumber, "Cannot append symbol "+*columnName+", which is possibly a duplicate.", RMLEvalMsgSeverity::Error);

      delete columnSpec;
   }
}

void RMLEval::scanSelectNodeForSymbols(RMLSymbolTable *pst, RMLSelectNode *node)
{
   node->symbolTable.parent=pst;

   // Scan the product space
   vector<RMLEvalExpNode *> *v=leftChainVector(node->left, OP_CROSSP);

   for (int i=((int)v->size())-1;i>=0;i--)
   {
      RMLEvalSpaceNode *p=(RMLEvalSpaceNode *)(*v)[i];

#if defined(PROJECT04)
      p->spaceIndex=(int)ss.size();
      ss.push_back(RMLSpaceRuntime(p));
#endif
      RMLSymbolSpace *spaceSymbol=node->symbolTable.addSpace(p->getSpaceName(), p->getSpaceAlias(), p->spaceIndex);

      if (spaceSymbol==nullptr)
         messageSet.appendMessage(node->lineNumber, "Cannot append space "+p->getSpaceDisplayName()+", which possibly causes potential ambiguities.", RMLEvalMsgSeverity::Error);

      // Scan the space if it is calculated
      if (!p->isExternal)
         scanSelectNodeForSymbols(&node->symbolTable, (RMLSelectNode *)p->left);

      if (spaceSymbol!=nullptr)
         populateSelectNodeSymbols(node, p);
   }

   delete v;

   // Scan the where expression
   scanExpNodeForSymbols(&node->symbolTable, node->right);

   // Additional semantic check is injected here
   RMLColumnList *columnList=node->columnList;
   int cc=columnList->columnCount();

   for (int i=0;i<cc;i++)
   {
      RMLColumnDef *columnDef=columnList->getColumnAt(i);

      //if (node->parent==nullptr)
      {
         string *columnAlias=columnDef->columnAlias;

         if (columnAlias!=nullptr && findLibFunction(*columnAlias)!=nullptr)
            messageSet.appendMessage(node->lineNumber, "Column alias "+*columnAlias+" is not allowed as it is a library function.", RMLEvalMsgSeverity::Error);
      }

      vector<RMLEvalExpNode *> *v=expressionPartVector(columnDef->rootNode);
      if (v->size()>1)
         messageSet.appendMessage(node->lineNumber, "Column defining expression must evaluate a single value.", RMLEvalMsgSeverity::Error);

      delete v;

      scanExpNodeForSymbols(&node->symbolTable, columnDef->rootNode);
   }
}

void RMLEval::scanExpNodeForSymbols(RMLSymbolTable *parentSymbolTable, RMLEvalExpNode *node)
{
   if (node->opCode==OP_SELECT)
      scanSelectNodeForSymbols(parentSymbolTable, (RMLSelectNode *)node);
   else
   {
      if (node->left!=nullptr)
         scanExpNodeForSymbols(parentSymbolTable, node->left);

      if (node->right!=nullptr)
         scanExpNodeForSymbols(parentSymbolTable, node->right);
   }
}

void RMLEval::scanCalculateTypes()
{
   int sc=stats->size();
   for (int i=0;i<sc;i++)
   {
      scanCalculateTypes((*stats)[i]);
   }
}

void RMLEval::scanCalculateTypes(RMLEvalExpNode *node)
{
   RMLEvalExpNode   *left=node->left,
                    *right=node->right;
   int               ec=0;
   RMLEvalSpaceNode *parentProcessingSpace;
   RMLSelectNode    *parentScopingSelectNode;

   if (node->opCode==OP_SELECT)
   {
      parentScopingSelectNode=scopingSelectNode;
      scopingSelectNode=(RMLSelectNode *)node;
   }
   else if (node->opCode==OP_SSPACE)
   {
      parentProcessingSpace=processingSpace;
      processingSpace=(RMLEvalSpaceNode *)node;
   }

   if (left!=nullptr)
   {
      scanCalculateTypes(left);
#if defined(PROJECT04)
      node->functionCallCount+=node->left->functionCallCount;
      node->nifc+=node->left->nifc;
      node->symRefCount+=node->left->symRefCount;
      node->selectCount+=node->left->selectCount;
      if (node->opCode!=OP_COMMA)
         node->forceActiveCount+=node->left->forceActiveCount;
#endif
   }

   if (right!=nullptr)
   {
      scanCalculateTypes(right);
#if defined(PROJECT04)
      node->functionCallCount+=node->right->functionCallCount;
      node->nifc+=node->right->nifc;
      node->symRefCount+=node->right->symRefCount;
      node->selectCount+=node->right->selectCount;
      if (node->opCode!=OP_COMMA)
         node->forceActiveCount+=node->right->forceActiveCount;
#endif
   }

   switch (node->opCode)
   {
      case OP_COMMA:
         node->type=node->left->type;
         break;
      case OP_BOR:
      case OP_BAND:
         if (left->type==right->type && left->type==RMLEvalType::RMLBoolean)
         {
            node->type=left->type;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> can be applied to booleans only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_EQ:
      case OP_NEQ:
         if (left->type==right->type)
         {
            node->type=RMLEvalType::RMLBoolean;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> can be applied to identical types only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_LTE:
      case OP_LT:
      case OP_GTE:
      case OP_GT:
         if (left->type==right->type)
         {
            if (left->type!=RMLEvalType::RMLBoolean)
               node->type=RMLEvalType::RMLBoolean;
            else
               messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> cannot be applied to booleans.", RMLEvalMsgSeverity::Error);
         }
         else
            messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> can be applied to identical types only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
         if (left->type==right->type)
         {
            if (left->type==RMLEvalType::RMLNumber)
               node->type=left->type;
            else
               messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> can be applied to numbers or strings only.", RMLEvalMsgSeverity::Error);
         }
         else
            messageSet.appendMessage(node->lineNumber, "Operator <"+RMLEval::opStr[node->opCode]+"> can be applied to identical types only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_ADD:
         if (left->type==RMLEvalType::RMLNumber && right->type==RMLEvalType::RMLNumber)
         {
            node->type=left->type;
         }
         else if (left->type==RMLEvalType::RMLString && right->type==RMLEvalType::RMLString)
         {
            node->type=left->type;
         }
         else if (left->type==RMLEvalType::RMLString && right->type==RMLEvalType::RMLNumber)
         {
            if (left->opCode==CONST && right->opCode==CONST)
            {
               node->right->stringValue=new string(to_string(node->right->doubleValue));
               node->right->type=RMLEvalType::RMLString;
            }
            node->type=left->type;
         }
         else if (left->type==RMLEvalType::RMLNumber && right->type==RMLEvalType::RMLString)
         {
            if (left->opCode==CONST && right->opCode==CONST)
            {
               node->left->stringValue=new string(to_string(node->left->doubleValue));
               node->left->type=RMLEvalType::RMLString;
            }
            node->type=right->type;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Operator (+) cannot be applied to the specified operands.", RMLEvalMsgSeverity::Error);
         break;
      case OP_COND:
         if (right->left->type==right->right->type)
         {
            node->type=
            right->type=right->left->type;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Ternary operator's 2nd and 3rd operands must be of the same type.", RMLEvalMsgSeverity::Error);
         break;
      case OP_MINUS:
         if (left->type==RMLEvalType::RMLNumber)
         {
            node->type=left->type;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Minus operator can be applied to numbers only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_NOT:
         if (left->type==RMLEvalType::RMLBoolean)
         {
            node->type=left->type;
         }
         else
            messageSet.appendMessage(node->lineNumber, "Not operator can be applied to booleans only.", RMLEvalMsgSeverity::Error);
         break;
      case OP_CALL:
         if (left!=nullptr)
         {
#if defined(PROJECT04)
            node->functionCallCount++;
#endif
            RMLFuncDesc *f=RMLEval::findLibFunction(*left->stringValue);

            if (f!=nullptr)
            {
               vector<RMLEvalExpNode *>   *v=expressionPartVector(right);
               int                        pc=v->size();

#if defined(PROJECT04)
               if (!f->idempotent)
                  node->nifc++;
               if (f->forceActive) node->forceActiveCount++;
#endif
               if (f->isAggregate && node->parent!=nullptr)
                  messageSet.appendMessage(node->lineNumber,
                        string("The aggregate function ")+f->name+" must be at root level of a column definition.",
                        RMLEvalMsgSeverity::Error);


               node->left->idNdx=f-RMLEval::lib;
               node->type=f->types[0];
               if (pc==f->paraCount)
               {
                  RMLEvalType    *td=f->types+1;

                  for (int i=0;i<pc;i++, td++)
                  {
                     if ((*v)[pc-i-1]->type!=*td)
                     {
                        messageSet.appendMessage(node->lineNumber,
                              string("Type of parameter ")+to_string(i+1)+" of function <"+*left->stringValue+"> must be "+RMLEval::typeStr[(int)*td]+".",
                              RMLEvalMsgSeverity::Error);
                        ec++;
                     }
                  }
               }
               else
               {
                  messageSet.appendMessage(node->lineNumber, "Function <"+*left->stringValue+"> requires "+to_string(f->paraCount)+" parameters. "+to_string(pc)+" supplied.", RMLEvalMsgSeverity::Error);
                  ec++;
               }
               if (ec==0)
                  node->pVector=v;
               else
                  delete v;
            }
            else
               messageSet.appendMessage(node->lineNumber, "Function <"+*left->stringValue+"> not found.", RMLEvalMsgSeverity::Error);
         }
         else
            if (right!=nullptr)
               node->type=right->type;
         break;
      case OP_CROSSP:
         node->type=RMLEvalType::RMLRowSet;
         break;
      case SYMREF:
         {
#if defined(PROJECT04)
            node->symRefCount++;
#endif
            RMLSymbolColumn *column=scopingSelectNode->symbolTable.resolveSymbol(node->stringValue!=nullptr?node->stringValue:nullptr, node->left!=nullptr?node->left->stringValue:nullptr);
            if (column!=nullptr)
            {
               node->type=column->type;
            }
            else
               messageSet.appendMessage(node->lineNumber, "Unresolved symbol <"+node->symRefString()+">.", RMLEvalMsgSeverity::Error);
         }
         break;
      case OP_SELECT:
#if defined(PROJECT04)
         if (scopingSelectNode->selectStatRuntimeIndex==-1)
         {
            int p=(int)ss.size();

            scopingSelectNode->selectStatRuntimeIndex=p;
            ss.push_back(RMLSpaceRuntime(scopingSelectNode));
            scopingSelectNode->simpleValueEvaluator=!node->parent || node->parent->opCode!=OP_SSPACE;
         }
#endif

         if (node->right->type!=RMLEvalType::RMLBoolean)
            messageSet.appendMessage(node->lineNumber, "Where condition expression must evaluate a boolean.", RMLEvalMsgSeverity::Error);

#if defined(PROJECT03)
         if (node->right->opCode==OP_COMMA)
            messageSet.appendMessage(node->lineNumber, "Where condition cannot have comma operator at root level.", RMLEvalMsgSeverity::Error);
#endif
         break;
   }

   if (node->opCode==OP_SSPACE)
      processingSpace=parentProcessingSpace;
   else if (node->opCode==OP_SELECT)
   {
      bool              backPatchTypes=parentScopingSelectNode!=nullptr && processingSpace!=nullptr && !processingSpace->isExternal;
      int               cc=scopingSelectNode->columnList->columnCount();
      RMLColumnList    *columnList=scopingSelectNode->columnList;
      RMLSymbolSpace   *symSpace=nullptr;
      RMLSymbolTable   *parentSymbolTable=nullptr;
#if defined(PROJECT04)
      node->selectCount++;
#endif
      if (backPatchTypes)
      {
         parentSymbolTable=&parentScopingSelectNode->symbolTable;
         symSpace=parentSymbolTable->findSpace(processingSpace->getSpaceName(), processingSpace->getSpaceAlias());
      }

      int aggregateCount=0;

      for (int i=0;i<cc;i++)
      {
         RMLColumnDef *def=columnList->getColumnAt(i);
         scanCalculateTypes(def->rootNode);
         node->functionCallCount+=def->rootNode->functionCallCount;
         node->nifc+=def->rootNode->nifc;
         node->symRefCount+=def->rootNode->symRefCount;
         node->selectCount+=def->rootNode->selectCount;
         if (node->opCode!=OP_COMMA)
            node->forceActiveCount+=def->rootNode->forceActiveCount;

         RMLEvalExpNode *testNode=def->rootNode;

         while (testNode->opCode==OP_COMMA)
            testNode=testNode->left;

         if (testNode->opCode==OP_CALL)
         {
            RMLFuncDesc  *func=RMLEval::lib+testNode->left->idNdx;
            if (func->isAggregate)
            {
   #if defined(PROJECT04)
               scopingSelectNode->aggregators.push_back(RMLEvalAggregator(testNode->left->idNdx, this, testNode));
   #endif
               aggregateCount++;
            }
            if (func->paraCount == 0) {
               testNode->right = new RMLEvalExpNode(testNode->lineNumber, 1.0, CONST);
            }
         }

#if defined(PROJECT04)
         if (cc>1 && def->rootNode->type!=RMLEvalType::RMLString)
         {
            //  Inject implicit cast operator to force string conversion
            RMLEvalExpNode *node=def->rootNode,
                           *typeCastNode=new RMLEvalExpNode(node->lineNumber, node->left, LRT, nullptr);

            typeCastNode->left=node;
            typeCastNode->left->parent=typeCastNode;

            typeCastNode->idNdx=LRT_NUMBERTOSTR;
            typeCastNode->type=RMLEvalType::RMLString;

            def->rootNode=typeCastNode;

         }
#endif
         if (backPatchTypes)
         {
            RMLSymbolColumn *symbolColumn=parentSymbolTable->findSymbol(symSpace, def->columnAlias);
            if (symbolColumn!=nullptr)
               symbolColumn->type=def->rootNode->type;
         }
      }

      if (aggregateCount>0 && aggregateCount<cc)
         messageSet.appendMessage(node->lineNumber, "When an aggregate column is specified, all remaining columns must be aggregate, too.", RMLEvalMsgSeverity::Error);

      if (cc==1)
      {
         RMLColumnDef *def=scopingSelectNode->columnList->getColumnAt(0);

         node->type=def->rootNode->type;
      }
      else
         node->type=RMLEvalType::RMLRowSet;

      scopingSelectNode=parentScopingSelectNode;
   }
}

void RMLEval::writeAsJSON(ostream *os)
{
   (*os)<<"{";

   (*os)<<"\"statements\": [";
   int sc=stats->size();

   if (sc>0)
   {
      (*stats)[0]->writeAsJSON(os);
      for (int i=1;i<sc;i++)
      {
         (*os) << ", ";
         (*stats)[i]->writeAsJSON(os);
      }
   }
   (*os) << "]";

#if defined(PROJECT04)
   if (ic!=nullptr)
   {
      (*os)<<", \"ic\": ";
      ic->writeAsJSON(os);
   }
#endif

   (*os)<<", \"messages\": ";
   messageSet.writeAsJSON(os);
   (*os)<<"}";
}

// RMLEvalExpNode
void RMLEvalExpNode::writeAsJSON(ostream *os)
{
   (*os)<<"{";

   (*os) <<"\"lineNumber\": "+to_string(lineNumber)+", \"nodeType\": \""+getNodeTypeStr()+"\", \"opCode\": "
         <<opCode<<", \"mnemonic\": \""<<RMLEval::opStr[opCode]
         <<"\", \"typeCode\": "<<((int)type)<<", \"type\": \""<<RMLEval::typeStr[(int)type] << "\""
         <<", \"idNdx\": "<<idNdx
         <<", \"functionCallCount\": " << functionCallCount
         <<", \"nifc\": " << nifc
         <<", \"symRefCount\": " << symRefCount
         <<", \"selectCount\": " << selectCount
         <<", \"forceActiveCount\": " << forceActiveCount;
   switch (opCode)
   {
      case CONST:
         switch (type)
         {
            case RMLEvalType::RMLString:
               (*os)<<", \"stringValue\": ";
               RMLEval::writeStrValue(os,stringValue);
               break;
            case RMLEvalType::RMLNumber:
               (*os)<<", \"numberValue\": \""<<doubleValue<<"\"";
               break;
            case RMLEvalType::RMLBoolean:
               (*os)<<", \"boolValue\": \""<<boolValue<<"\"";
               break;
            default:
               break;
         }
         break;
      case INSID:
      case INSMEM:
      case INSFREF:
      case OP_SSPACE:
         if (stringValue!=nullptr)
            (*os)<<", \"id\": \""<<*stringValue<<"\"";
         break;
      case SYMREF:
         if (stringValue!=nullptr)
            (*os)<<", \"id\": \""<<*stringValue<<"\"";
         (*os)<<", \"fqn\": \""<<symRefString()<<"\"";

   }

   if (left!=nullptr)
   {
      (*os)<<", \"left\":";
      left->writeAsJSON(os);
   }

   if (right!=nullptr)
   {
      (*os)<<", \"right\":";
      right->writeAsJSON(os);
   }

   writeAdditionalMembers(os);

   (*os)<<"}";
}

const char *RMLEvalExpNode::getNodeTypeStr()
{
   return "expression node";
}

void RMLEvalExpNode::writeAdditionalMembers(ostream *os)
{
}

void RMLSelectNode::writeAdditionalMembers(ostream *os)
{
   (*os)<<", \"columnList\":[";

   int cc=columnList->columnCount();

   for (int i=0;i<cc;i++)
   {
      RMLColumnDef *columnDef=columnList->getColumnAt(i);

      if (i>0)
         (*os)<<", ";

      (*os)<<"{\"id\":\""+(*columnDef->columnAlias)+"\", \"expression\":";

      columnDef->rootNode->writeAsJSON(os);

      (*os)<<"}";
   }

   (*os)<<"]";

   cc=symbolTable.symbolCount();
   (*os)<<", \"symbolList\":[";

   for (int i=0;i<cc;i++)
   {
      RMLSymbolColumn *sym=symbolTable.symbolAt(i);

      if (i>0)
         (*os)<<", ";

      (*os)<<"{\"id\":\""+sym->fqn()+"\", \"type\":\""+RMLEval::typeStr[(int)sym->type]+"\"}";
   }

   (*os)<<"]";
}

