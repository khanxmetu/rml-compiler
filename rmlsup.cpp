
using namespace std;
// #include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "rmlsup.h"

#include "rmlsyn.tab.hh"
#include "MyParser.h"
#include "FlexLexer.h"

int *RMLEval::sVector=nullptr;

string RMLEval::opStr[]=
{
	"nop",
	"comma",
	"assign",
	"cond",
	"alt",
	"band",
	"bor",
	"eq",
	"neq",
	"lt",
	"lte",
	"gt",
	"gte",
	"add",
	"sub",
	"mul",
	"div",
	"minus",
	"not",
	"space",
	"select",
	"call",
	"crossp",
	"jmp",
	"jf",
	"jt",
	"symref",
	"const",
	"lrt",
	"pop",
	"insid",
	"insmem",
	"insfref"
};

string RMLEval::typeStr[]=
{
   "none",
   "number",
   "string",
   "boolean",
   "rowset"
};

string RMLEval::severityStr[]=
{
   "Error",
   "Warning",
   "Info"
};

string RMLEval::lrtNames[]=
{
   "LRT_RESETSCANNER",
   "LRT_ADVANCESCANNER",
   "LRT_AGGREGATE",
   "LRT_SNAPSHOT",
   "LRT_SETWORDBASE",
   "LRT_CREATERESULTSET",
   "LRT_FINALIZERESULTSET",
   "LRT_NUMBERTOSTR",
   "LRT_STRCMP",
   "LRT_CATSTRING",
   "LRT_ALLOCATESTRING",
   "LRT_POSTEXECUTECLEANUP",
   "LRT_CHECKEXCEPTION"
};

bool RMLTypeDesc::operator == (RMLTypeDesc other)
{
   return type==other.type && dim==other.dim;
}
bool RMLTypeDesc::operator != (RMLTypeDesc other)
{
   return type!=other.type || dim!=other.dim;
}

RMLFuncDesc RMLEval::lib[]=
{
   {"stddev", (void *)RMLEval::stddev, false, true, true, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"mean", (void *)RMLEval::mean, false, true, true, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"count", (void *)RMLEval::count, false, true, true, true, false, 0, {RMLEvalType::RMLNumber}},
   {"min", (void *)RMLEval::min, false, true, true, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"max", (void *)RMLEval::max, false, true, true, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"sum", (void *)RMLEval::sum, false, true, true, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"sin", (void *)RMLEval::sin, false, false, false, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"cos", (void *)RMLEval::cos, false, false, false, true, false, 1, {RMLEvalType::RMLNumber,RMLEvalType::RMLNumber}},
   {"tan", (void *)&RMLEval::tan, false, false, true, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"pi", (void *)RMLEval::pi, false, false, false, true, false, 0, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"atan", (void *)RMLEval::atan, false, false, false, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"asin", (void *)RMLEval::asin, false, false, false, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"acos", (void *)RMLEval::acos, false, false, false, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"exp", (void *)RMLEval::exp, false, false, false, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"ln", (void *)RMLEval::ln, true, false, true, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"number", (void *)RMLEval::number, true, false, true, true, false, 1,{RMLEvalType::RMLNumber, RMLEvalType::RMLString}},
   {"random", (void *)RMLEval::random, false, false, false, false, false, 1,{RMLEvalType::RMLNumber, RMLEvalType::RMLNumber}},
   {"print", (void *)RMLEval::print, false, false, false, true, true, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLString}},
   {"len", (void *)RMLEval::len, false, false, false, true, false, 1, {RMLEvalType::RMLNumber, RMLEvalType::RMLString}},
   {"right", (void *)RMLEval::right, true, false, false, true, false, 2, {RMLEvalType::RMLString, RMLEvalType::RMLString, RMLEvalType::RMLNumber}},
   {"left", (void *)RMLEval::left, true, false, false, true, false, 2, {RMLEvalType::RMLString, RMLEvalType::RMLString, RMLEvalType::RMLNumber}}
};

RMLColumnDef::RMLColumnDef(RMLEvalExpNode *pRootNode, string *pColumnAlias)
{
   rootNode=pRootNode;
   columnAlias=pColumnAlias;
}

RMLColumnDef::~RMLColumnDef()
{
   delete rootNode;
   delete columnAlias;
}

RMLColumnList::RMLColumnList(RMLColumnDef *pColumnDef)
{
   addColumn(pColumnDef);
}

void RMLColumnList::addColumn(RMLColumnDef *pColumnDef)
{
   columns.push_back(pColumnDef);
}

int RMLColumnList::columnCount()
{
   return columns.size();
}

RMLColumnDef *RMLColumnList::getColumnAt(int ndx)
{
   if (ndx>=0 && ndx<(int)columns.size())
      return columns[ndx];

   return nullptr;
}

RMLColumnList::~RMLColumnList()
{
   for (int i=columns.size()-1;i>=0;i--)
      delete columns[i];
}

RMLEvalNode::RMLEvalNode(int pLineNumber)
{
   lineNumber=pLineNumber;
}

RMLEvalNode::~RMLEvalNode()
{
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *pLeft, int pOpCode)
   :RMLEvalNode(pLineNumber)
{
   if (pLeft!=nullptr)
      pLeft->parent=this;

   left=pLeft;
   opCode=pOpCode;
   right=nullptr;
}

string RMLEvalExpNode::symRefString()
{
   if (stringValue!=nullptr)
      return string(*stringValue)+"."+string(*left->stringValue);

   return string(*left->stringValue);
}

RMLColumnSpec::RMLColumnSpec(string *pName, RMLEvalType pType, int pColumnNdx)
{
   name=pName;
   type=pType;
   columnNdx=pColumnNdx;
}

RMLColumnSpec::~RMLColumnSpec()
{
}

RMLSelectNode::RMLSelectNode(int pLineNumber, RMLColumnList *pColumnList, RMLEvalExpNode *pLeft, int pOpCode, RMLEvalExpNode *pRight)
   : RMLEvalExpNode(pLineNumber, pLeft, pOpCode, pRight)
{
   columnList=pColumnList;
   type=RMLEvalType::RMLRowSet;
}

int RMLSelectNode::findColumnNdx(string *fieldId)
{
   int retVal=-1;
   int cnt=columnList->columnCount();

   for (int i=0;i<cnt;i++)
   {
      RMLColumnDef *def=columnList->getColumnAt(i);

      if (def->columnAlias->compare(*fieldId)==0)
         if (retVal>-1)
         {
            retVal=-1;
            break;
         }
         else
            retVal=i;
   }
   return retVal;
}

RMLColumnSpec *RMLSelectNode::columnAt(int ndx)
{
   RMLColumnDef *def=columnList->getColumnAt(ndx);

   if (def!=nullptr)
      return new RMLColumnSpec(def->columnAlias, def->rootNode->type, ndx);

   return nullptr;
}

/*
RMLColumnSpec *RMLSelectNode::resolveSymbol(RMLEvalSpaceNode *spaceNode, string *spaceOrAlias, string *columnId)
{
   if (spaceNode->stringValue->compare(*spaceOrAlias))
   {
      return spaceNode->findColumn(columnId);
   }

   return nullptr;
}

RMLColumnSpec *RMLSelectNode::resolveSymbol(string *spaceOrAlias, string *columnId)
{
   RMLEvalExpNode *scanner=left;
   RMLColumnSpec *colSpec=nullptr;
   int fc=0;

   while (scanner && scanner->opCode==OP_CROSSP)
   {
      colSpec=resolveSymbol((RMLEvalSpaceNode *)scanner, spaceOrAlias, columnId);
      if (colSpec!=nullptr)
         fc++;

      scanner=scanner->left;
   }
   colSpec=resolveSymbol((RMLEvalSpaceNode *)scanner, spaceOrAlias, columnId);
   if (colSpec!=nullptr)
      fc++;

   if (fc!=1)
      colSpec=nullptr;

   return colSpec;
}
*/

const char *RMLSelectNode::getNodeTypeStr()
{
   return "select node";
}

RMLSelectNode::~RMLSelectNode()
{
   if (columnList!=nullptr)
      delete columnList;
}

RMLEvalSpaceNode::RMLEvalSpaceNode(int pLineNumber, string *pExternalSpaceName, RMLEvalExpNode *alias)
                 :RMLEvalExpNode(pLineNumber, pExternalSpaceName, OP_SSPACE)
{
   right=alias;
   isExternal=true;
   type=RMLEvalType::RMLRowSet;
}

RMLEvalSpaceNode::RMLEvalSpaceNode(int pLineNumber, RMLSelectNode *pSelectNode, RMLEvalExpNode *alias)
                 :RMLEvalExpNode(pLineNumber, pSelectNode, OP_SSPACE, alias)
{
   isExternal=false;
   type=RMLEvalType::RMLRowSet;
}

string *RMLEvalSpaceNode::getSpaceName()
{
   if (stringValue!=nullptr)
      return stringValue;

   return nullptr;
}

string *RMLEvalSpaceNode::getSpaceAlias()
{
   if (right!=nullptr)
      return right->stringValue;

   return nullptr;
}

string RMLEvalSpaceNode::getSpaceDisplayName()
{
   if (stringValue!=nullptr && right!=nullptr)
      return string(*stringValue)+string(*right->stringValue);

   if (stringValue!=nullptr)
      return string(*stringValue);

   return string(*right->stringValue);
}

RMLEvalExternalSpaceNode::RMLEvalExternalSpaceNode(int pLineNumber, string *pExternalSpaceName, RMLEvalExpNode *alias)
     : RMLEvalSpaceNode(pLineNumber, pExternalSpaceName, alias)
{
   string fileName=(*pExternalSpaceName)+".csv";

   csv=new CSVProxy(&fileName);
}

RMLEvalExternalSpaceNode::~RMLEvalExternalSpaceNode()
{
   delete csv;

   // This ascendant does not delete this value!
   // So, delete it!
   if (stringValue!=nullptr)
      delete stringValue;
}

int RMLEvalExternalSpaceNode::columnCount()
{
   int retVal=-1;

   if (csv!=nullptr)
      retVal=csv->columnCount();

   return retVal;
}

RMLColumnSpec *RMLEvalExternalSpaceNode::columnAt(int ndx)
{
   if (csv!=nullptr)
   {
      string *columnName=csv->columnName(ndx);

      if (columnName!=nullptr)
         return new RMLColumnSpec(columnName, RMLEvalType::RMLString, ndx);
   }
   return nullptr;
}

RMLColumnSpec *RMLEvalExternalSpaceNode::findColumn(string *fieldId)
{
   int ndx=csv->findColumnNdx(fieldId);

   if (ndx>-1)
      return new RMLColumnSpec(fieldId, RMLEvalType::RMLString, ndx);

   return nullptr;
}

const char *RMLEvalExternalSpaceNode::getNodeTypeStr()
{
   return "external space";
}

RMLEvalCalculatedSpaceNode::RMLEvalCalculatedSpaceNode(int pLineNumber, RMLSelectNode *pSelectNode, RMLEvalExpNode *alias)
     : RMLEvalSpaceNode(pLineNumber, pSelectNode, alias)
{
   pSelectNode->calculatingSpace=this;
}

int RMLEvalCalculatedSpaceNode::columnCount()
{
   int retVal=-1;
   RMLSelectNode *selectNode=(RMLSelectNode *)left;

   if (selectNode!=nullptr && selectNode->columnList!=nullptr)
      retVal=selectNode->columnList->columnCount();

   return retVal;
}

RMLColumnSpec *RMLEvalCalculatedSpaceNode::columnAt(int ndx)
{
   RMLSelectNode *spaceNode=(RMLSelectNode *)left;
   RMLColumnSpec *cs=spaceNode->columnAt(ndx),
                 *r=nullptr;

   if (cs!=nullptr)
   {
      r=new RMLColumnSpec(cs->name, cs->type, ndx);
      delete cs;
   }

   return r;
}

RMLColumnSpec *RMLEvalCalculatedSpaceNode::findColumn(string *fieldId)
{
   RMLSelectNode *spaceNode=(RMLSelectNode *)left;
   int ndx=spaceNode->findColumnNdx(fieldId);

   if (ndx>-1)
   {
      RMLColumnDef *def=spaceNode->columnList->getColumnAt(ndx);
      return new RMLColumnSpec(fieldId, def->rootNode->type, ndx);
   }

   return nullptr;
}

const char *RMLEvalCalculatedSpaceNode::getNodeTypeStr()
{
   return "calculated space";
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, double value, int pOpCode)
   :RMLEvalNode(pLineNumber)
{
   left=
   right=nullptr;
   opCode=pOpCode;
   doubleValue=value;
   type=RMLEvalType::RMLNumber;
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, string *p1, RMLEvalExpNode *child, int pOpCode)
   :RMLEvalNode(pLineNumber)
{
   left=child;
   right=nullptr;
   opCode=pOpCode;
   stringValue=p1;
   if (opCode==CONST)
	  type=RMLEvalType::RMLString;
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, string *value, int pOpCode)
   :RMLEvalNode(pLineNumber)
{
   left=
   right=nullptr;
   opCode=pOpCode;
   stringValue=value;
   if (opCode==CONST)
      type=RMLEvalType::RMLString;
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, bool value, int pOpCode)
   :RMLEvalNode(pLineNumber)
{
   left=
   right=nullptr;
   opCode=pOpCode;
   boolValue=value;
   type=RMLEvalType::RMLBoolean;
}


RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *rowSetRoot)
   :RMLEvalNode(pLineNumber)
{
   rowSetRoot->parent=this;
   left=rowSetRoot;
   right=nullptr;
   opCode=CONST;
   type=RMLEvalType::RMLRowSet;
}

RMLEvalExpNode::~RMLEvalExpNode()
{
   if (((opCode==CONST && type==RMLEvalType::RMLString)
        || opCode==SYMREF
        || opCode==INSFREF
        || opCode==INSID
        || opCode==INSMEM
        || (opCode==LRT && idNdx==LRT_ALLOCATESTRING))
       && stringValue!=nullptr)
      delete stringValue;

   if (pVector!=nullptr)
      delete pVector;

   if (left!=nullptr)
      delete left;

   if (right!=nullptr)
      delete right;
}

RMLEval::RMLEval(MyParser *pParser)
{
   statementInScan=nullptr;
   stats=&pParser->stats;

#if defined(PROJECT04)
   ic=new RMLIC();
#endif

   parser=pParser;
   optimization=pParser->optimization;

   if (RMLEval::sVector==nullptr)
   {
      int s=sizeof(RMLEval::lib)/sizeof(RMLFuncDesc);
      RMLEval::sVector=new int[s];

      for (int i=0;i<s;i++)
         RMLEval::sVector[i]=i;

      qsort(RMLEval::sVector, s, sizeof(int), RMLEval::compareFDesc);
   }
}

RMLEvalExpNode::RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *pLeft, int pOpCode, RMLEvalExpNode *pRight)
   :RMLEvalNode(pLineNumber)
{
   left=pLeft;
   opCode=pOpCode;
   right=pRight;

   if (pLeft!=nullptr)
      pLeft->parent=this;
   if (pRight!=nullptr)
      pRight->parent=this;
}

RMLEval::~RMLEval()
{
#if defined(DO_X64)
   if (exceptionWord!=nullptr)
      delete exceptionWord;
#endif

   if (ic!=nullptr)
      delete ic;
}

bool RMLEval::nodeDiscardable(RMLEvalExpNode *node)
{
   return node->forceActiveCount==0 && node->selectCount==0;
}

void RMLEval::writeStrValue(ostream *os, string *strValue)
{
   static  unsigned char const *hd=(unsigned char const *)"0123456789abcdef";
   string  t=string();
   int     l=(int)strValue->length();

   t+='"';
   for (int i=0;i<l;i++)
   {
      char c=(*strValue)[i];

      switch (c)
      {
         case '\n':
            t+="\\n";
            break;
         case '\t':
            t+="\\t";
            break;
         case '\r':
            t+="\\r";
            break;
         case '\\':
            t+="\\\\";
            break;
         default:
            if (c<32)
            {
               t+="\\x";
               t+=hd[c>>4];
               t+=hd[c&0xf];
            }
            else
               t+=(*strValue)[i];
      }
   }

   t+='"';
   (*os) << t;
}

int RMLEval::compareFDesc(const void *v1, const void *v2)
{
   int   i1=*(int*)v1,
         i2=*(int*)v2;

   return RMLEval::lib[i1].name.compare(RMLEval::lib[i2].name);
}

RMLFuncDesc *RMLEval::findLibFunction(string fName)
{
   int   p=0,
         q=sizeof(RMLEval::lib)/sizeof(RMLFuncDesc)-1;

   do
   {
      int               n=(p+q)/2;
      RMLFuncDesc      *desc=RMLEval::lib+RMLEval::sVector[n];
      int               test=desc->name.compare(fName);

      if  (test==0)
         return desc;

      if (test>0)
         q=n-1;
      else
         p=n+1;

   } while (p<=q);

   return nullptr;
}

RMLFuncDesc *RMLEval::libFunctionAt(int ndx)
{
   return lib+ndx;
}

vector<RMLEvalExpNode *> *RMLEval::expressionPartVector(RMLEvalExpNode *node)
{
   return leftChainVector(node, OP_COMMA);
}

vector<RMLEvalExpNode *> *RMLEval::leftChainVector(RMLEvalExpNode *node, int opCode)
{
   vector<RMLEvalExpNode *> *retVal=new vector<RMLEvalExpNode *>();

   if (node!=nullptr)
   {
      while (node->opCode==opCode)
      {
         retVal->push_back(node->right);
         node=node->left;
      }

      retVal->push_back(node);
   }

   return retVal;
}

#if defined(PROJECT04)
void RMLEval::registerStringObject(string *str)
{
   stringVectorToClean.push_back(str);
}

void RMLEval::registerRMLRowSetObject(RMLEvalRowSet *arr)
{
   //arrayVectorToClean.push_back(arr);
}
#endif

RMLSymbolSpace::RMLSymbolSpace(string *pSpaceName, string *pAlias, int pSpaceIndex)
{
   spaceName=pSpaceName;
   alias=pAlias;
   spaceIndex=pSpaceIndex;
}

RMLSymbolSpace::~RMLSymbolSpace()
{
   /*
   if (spaceName!=nullptr)
      delete spaceName;

   if (alias!=nullptr)
      delete alias;*/
}

RMLSymbolColumn::RMLSymbolColumn(RMLSymbolSpace *pSpace, string *pName, RMLEvalType pType)
{
   space=pSpace;
   name=pName;
   type=pType;
}

RMLSymbolColumn::~RMLSymbolColumn()
{
   /*
   if (space!=nullptr)
      delete space;
   */

   //delete name;
}

string RMLSymbolColumn::fqn()
{
   string retVal=string("[");

   if (space!=nullptr)
   {
      if (space->spaceName!=nullptr)
         retVal+=*space->spaceName;

      if (space->alias!=nullptr)
      {
         if (space->spaceName!=nullptr)
            retVal+=" ";

         retVal+=*space->alias;
      }
   }

   return retVal+"."+*name+"]";
}

RMLSymbolTable::RMLSymbolTable()
{
}

RMLSymbolTable::~RMLSymbolTable()
{
   auto                 c=columns.begin();
   size_t               cnt=columns.size();

   for (size_t i=0;i<cnt;i++, c++)
      delete *c;

   auto                 s=spaces.begin();
   cnt=spaces.size();

   for (size_t i=0;i<cnt;i++, s++)
      delete *s;
}

int RMLSymbolTable::spaceCount()
{
   return spaces.size();
}

RMLSymbolSpace *RMLSymbolTable::spaceAt(int i)
{
   if (i<(int)spaces.size())
      return spaces[i];

   return nullptr;
}

RMLSymbolSpace *RMLSymbolTable::findSpace(string *name)
{
   int cnt=spaces.size();

   if (name!=nullptr)
      for (int i=0;i<cnt;i++)
      {
         RMLSymbolSpace *symbolSpace=spaces[i];

         if (
               (symbolSpace->spaceName!=nullptr && symbolSpace->spaceName->compare(*name)==0)
               || (symbolSpace->alias!=nullptr && symbolSpace->alias->compare(*name)==0)
            )
            return symbolSpace;
      }

   return nullptr;
}

RMLSymbolSpace *RMLSymbolTable::findSpace(string *spaceName, string *alias)
{
   int cnt=spaces.size();

   for (int i=0;i<cnt;i++)
   {
      RMLSymbolSpace *symbolSpace=spaces[i];

      if (
         ((spaceName==nullptr && symbolSpace->spaceName==nullptr) ||
          (spaceName!=nullptr && symbolSpace->spaceName!=nullptr && spaceName->compare(*symbolSpace->spaceName)==0)) &&
         ((alias==nullptr && symbolSpace->alias==nullptr) ||
          (alias!=nullptr && symbolSpace->alias!=nullptr && alias->compare(*symbolSpace->alias)==0))
         )
         return symbolSpace;
   }

   return nullptr;
}

RMLSymbolSpace *RMLSymbolTable::addSpace(string *spaceName, string *alias, int spaceIndex)
{
   RMLSymbolSpace *sSpace=findSpace(spaceName),
                  *aSpace=findSpace(alias),
                  *ss=nullptr;

   if (aSpace==nullptr)
   {
      if (
          sSpace==nullptr ||
          (alias!=nullptr  && sSpace->alias==nullptr) ||
          (alias==nullptr  && sSpace->alias!=nullptr) ||
          sSpace->alias->compare(*alias)!=0
         )
      {
         ss=new RMLSymbolSpace(spaceName, alias, spaceIndex);
         spaces.push_back(ss);
      }
      // else A space having the same name or alias prevents defining the space
   }
   // else Duplicate symbol by alias

   return ss;
}

RMLSymbolColumn *RMLSymbolTable::symbolAt(int ndx)
{
   if (ndx>=0 && ndx<(int)columns.size())
      return columns[ndx];

   return nullptr;
}

int RMLSymbolTable::symbolCount()
{
   return (int)columns.size();
}

RMLSymbolColumn *RMLSymbolTable::findSymbol(RMLSymbolSpace *space, string *name)
{
   size_t          cnt=columns.size();

   for (size_t i=0;i<cnt;i++)
   {
      RMLSymbolColumn *d=columns[i];

      if (d->space==space && name->compare(*d->name)==0)
         return d;
   }
;   return nullptr;
}

RMLSymbolColumn *RMLSymbolTable::findSymbol(string *spaceName, string *name)
{
   RMLSymbolSpace *space=findSpace(spaceName);

   if (space!=nullptr)
      return findSymbol(space, name);

   return nullptr;
}

RMLSymbolColumn *RMLSymbolTable::addSymbol(string *spaceName, string *alias, string *name, RMLEvalType pType)
{
   RMLSymbolColumn *symbolColumn=nullptr;
   RMLSymbolSpace *space=findSpace(spaceName, alias);

   if (space!=nullptr)
   {
      symbolColumn=findSymbol(space, name);

      if (symbolColumn==nullptr)
      {
         symbolColumn=new RMLSymbolColumn(space, name, pType);

         symbolColumn->ndx=columns.size();
         columns.push_back(symbolColumn);
      }
      else
         symbolColumn=nullptr;
   }

   return symbolColumn;
}

RMLSymbolColumn *RMLSymbolTable::resolveSymbol(string *spaceName, string *name)
{
   RMLSymbolColumn *retVal=nullptr;
   if (spaceName==nullptr)
   {
      int cnt=spaces.size();
      for (int i=0;i<cnt;i++)
      {
         RMLSymbolColumn *test=findSymbol(spaces[i], name);
         if (test!=nullptr)
         {
            if (retVal==nullptr)
               retVal=test;
            else
            {
               retVal=nullptr;
               break;
            }
         }
      }
   }
   else
   {
      RMLSymbolSpace *space=findSpace(spaceName);

      if (space!=nullptr)
         retVal=findSymbol(space, name);
   }

   if (retVal==nullptr && parent!=nullptr)
      retVal=parent->resolveSymbol(spaceName, name);

   return retVal;
}

RMLMsgDesc::RMLMsgDesc(int pLineNumber, string pMsg, RMLEvalMsgSeverity pS)
{
   lineNumber=pLineNumber;
   msg=pMsg;
   s=pS;
}

RMLMsgSet::RMLMsgSet()
{
}

RMLMsgSet::~RMLMsgSet()
{
   auto                 c=messages.begin();
   size_t               cnt=messages.size();

   for (size_t i=0;i<cnt;i++, c++)
      delete *c;
}

void RMLMsgSet::appendMessage(int pLineNumber, string pMsg, RMLEvalMsgSeverity pS)
{
   messages.push_back(new RMLMsgDesc(pLineNumber, pMsg, pS));
}

bool RMLMsgSet::compareMessageLines(RMLMsgDesc *p1, RMLMsgDesc *p2)
{
   return p1->lineNumber < p2->lineNumber;
}

void RMLMsgSet::sort()
{
   if (messages.size()>0)
      std::sort(messages.begin(), messages.end(), compareMessageLines);
}

int RMLMsgSet::messageCount(RMLEvalMsgSeverity t)
{
   size_t               cnt=messages.size();
   int                  retVal=0;

   for (size_t i=0;i<cnt;i++)
      if (messages[i]->s==t)
         retVal++;

   return retVal;
}

#if defined(PROJECT04)
RMLICInst::RMLICInst(int pOpCode, int pP1)
{
   opCode=pOpCode;
   p1=pP1;
   type=RMLEvalType::RMLNone;
}

RMLICInst::RMLICInst(int pOpCode, int pP1, RMLEvalType pType)
{
   opCode=pOpCode;
   p1=pP1;
   type=pType;
}

RMLICInst::RMLICInst()
{
}

RMLICInst::~RMLICInst()
{
}

RMLCodeLocation::RMLCodeLocation()
{
}

RMLCodeLocation::RMLCodeLocation(RMLIC *pIc, RMLICInst i)
{
   ic=pIc;
   inst=i;
   relocation=0;
}

RMLCodeLocation::~RMLCodeLocation()
{
   if (ij!=nullptr)
      delete ij;
}

void RMLCodeLocation::registerPredecessor(int source)
{
   if (ij==nullptr)
      ij=new vector<int>();

   int cnt=ij->size();

   for (int i=0;i<cnt;i++)
      if ((*ij)[i]==source)
         return;

   ij->push_back(source);
}

bool RMLCodeLocation::ivHasNdx(vector<int> *v, int ndx)
{
   int cnt=v->size(),
       i,
       *p=v->data();

   for (i=0;i<cnt;i++, p++)
      if (*p==ndx)
         break;

   return i<cnt;
}

void RMLCodeLocation::ivAppend(vector<int> *v, int ndx)
{
   if (!ivHasNdx(v, ndx))
      v->push_back(ndx);
}
void RMLCodeLocation::ivAppend(vector<int> *v, vector<int> *s)
{
   int cnt=s->size();

   for (int i=0;i<cnt;i++)
      ivAppend(v,(*s)[i]);
}

void RMLCodeLocation::predecessors(vector<int> *accumulator)
{
   int ndx=ic->codeLocationIndex(this);

   if (ij!=nullptr)
      ivAppend(accumulator,ij);

   if (ndx>0)
   {
      RMLCodeLocation *precedingCL=(this-1);

      if (precedingCL->inst.opCode!=JMP)
         ivAppend(accumulator, ndx-1);
   }
}

vector<int> *RMLCodeLocation::predecessors()
{
   vector<int> *retVal=new vector<int>();

   predecessors(retVal);

   return retVal;
}

void RMLIC::collectImmediateJumps()
{
   RMLCodeLocation     *cl=code.data();
   int                  cnt=code.size();

   for (int i=0;i<cnt;i++, cl++)
   {
      RMLICInst     *inst=&cl->inst;
      int            opCode=inst->opCode;

      if (opCode==JMP || opCode==JF || opCode==JT)
         code[inst->p1].registerPredecessor(i);
   }
}

RMLCodePathWindow::RMLCodePathWindow()
{
}

RMLCodePathWindow::~RMLCodePathWindow()
{
   int               cnt=path.size();
   RMLCodePath     **p=path.data();

   for (int i=0;i<cnt;i++, p++)
      delete *p;
}

void RMLCodePathWindow::build(RMLIC *ic, int codeLocationIndex, int windowSize)
{
   int *builder=new int[windowSize];

   build(ic, codeLocationIndex, windowSize, 0, builder);

   delete []builder;
}

void RMLCodePathWindow::build(RMLIC *ic, int codeLocationIndex, int windowSize, int depth, int *builder)
{
   RMLCodeLocation *cl=ic->codeLocationAt(codeLocationIndex);
   vector<int>   *t=cl->predecessors();
   int            cnt=t->size();

   builder[depth]=codeLocationIndex;
   if (cnt>0 && depth<windowSize-1)
   {
      for (int i=0;i<cnt;i++)
         build(ic, (*t)[i], windowSize, depth+1, builder);
   }
   else
   {
      RMLCodePath *p=new vector<int>();

      for (int i=depth;i>=0;i--)
         p->push_back(builder[i]);

      path.push_back(p);
   }

   delete t;
}

RMLIC::RMLIC()
{
}

RMLIC::~RMLIC()
{
}

RMLICInst *RMLIC::emitIC(int opCode, int p1)
{
   code.push_back(RMLCodeLocation(this, RMLICInst(opCode, p1)));
   return &code[code.size()-1].inst;
}

RMLICInst *RMLIC::emitIC(int opCode, int p1, RMLEvalType type)
{
   code.push_back(RMLCodeLocation(this, RMLICInst(opCode, p1, type)));
   return &code[code.size()-1].inst;
}

RMLICInst *RMLIC::instructionAt(int ndx)
{
   if (ndx>=0 && ndx<(int)code.size())
      return &code[ndx].inst;

   return nullptr;
}

RMLCodeLocation *RMLIC::codeLocationAt(int ndx)
{
   if (ndx>=0 && ndx<(int)code.size())
      return &code[ndx];

   return nullptr;
}

bool RMLIC::markRemoval(int ndx, int count)
{
   bool retVal=ndx>=0 && ndx+count<=(int)code.size();

   if (retVal)
   {

      int   e=ndx+count;

      for (int i=ndx;i<e;i++)
         code[i].inst.opCode=OP_NOP;
   }

   return retVal;
}

int RMLIC::applyRemoval()
{
   int                     cnt=code.size(),
                           offset=0;
   RMLCodeLocation        *cl=code.data();

   for (int i=0;i<cnt;i++, cl++)
   {
      cl->relocation=offset;
      if (cl->inst.opCode==OP_NOP)
         offset--;
   }

   cl=code.data();
   for (int i=0;i<cnt;i++, cl++)
   {
      RMLICInst  *inst=&cl->inst;

      switch (inst->opCode)
      {
         case JMP:
         case JF:
         case JT:
            inst->p1+=code[inst->p1].relocation;
            break;
      }

      if (cl->ij!=nullptr)
      {
         vector<int> *ij=cl->ij;
         int *p=ij->data(),
              cnt=ij->size();

         for (int i=0;i<cnt;i++, p++)
            *p=code[*p].relocation;
      }

      int r=code[i].relocation;
      code[i+r].inst=*inst;
   }

   code.resize(cnt+offset);

   return offset;
}

int RMLIC::instCount()
{
   return code.size();
}

int RMLIC::codeLocationIndex(RMLCodeLocation *cl)
{
   return cl-code.data();
}

void RMLIC::writeReadable(ostream *outStream)
{
   //if (code!=nullptr)
   {
      char buf[128];
      int cnt=code.size();

      for (int i=0;i<cnt;i++)
      {
        RMLICInst *inst=&code[i].inst;

        sprintf(buf,"%5.5d %-8s %2d type:[%s] ", i, RMLEval::opStr[inst->opCode].c_str(), inst->p1, RMLEval::typeStr[(int)inst->type].c_str());

        (*outStream) << buf;

        if (inst->opCode==CONST)
        {
          switch (inst->type)
          {
            case RMLEvalType::RMLNumber:
               (*outStream) << inst->numConstant;
               break;
            case RMLEvalType::RMLBoolean:
               (*outStream) << inst->boolConstant;
               break;
            case RMLEvalType::RMLString:
               RMLEval::writeStrValue(outStream, inst->strConstant);
               break;
            default: ;
          }
        }

        if (inst->opCode==INSMEM)
        {
          if (inst->strConstant!=nullptr)
            RMLEval::writeStrValue(outStream, inst->strConstant);
        }

         if (inst->opCode==LRT)
         {
            switch (inst->p1)
            {
               case LRT_ALLOCATESTRING:
                  RMLEval::writeStrValue(outStream, inst->strConstant);
                  break;
               case LRT_RESETSCANNER:
               case LRT_ADVANCESCANNER:
               case LRT_AGGREGATE:
               case LRT_SNAPSHOT:
               case LRT_CREATERESULTSET:
               case LRT_FINALIZERESULTSET:
               case LRT_STRCMP:
                  (*outStream) << inst->intConstant;
            }

            (*outStream) << " ; " << RMLEval::lrtNames[inst->p1] ;
         }

         if (inst->opCode==OP_CALL)
           (*outStream) << "\"" << RMLEval::libFunctionAt((int)inst->numConstant)->name << "\" @"<< (int)inst->numConstant;

         (*outStream) << endl;
      }
   }
}
#endif

RMLEvalExpNode *RMLEval::destroyExpTree(RMLEvalExpNode *node)
{
   delete node;

   return nullptr;
}

#if defined(PROJECT04)

void RMLEval::transformStrConstToLRT(RMLEvalExpNode *node)
{
   if (node!=nullptr && node->type==RMLEvalType::RMLString && node->opCode==CONST)
   {
      node->opCode=LRT;
      node->idNdx=LRT_ALLOCATESTRING;
   }
}

uint64_t RMLEval::readLine(RMLEval *rmlEval, RMLSpaceRuntime *srt)
{
   CSVProxy *csv=srt->cp;
   CSVLine  *line=csv->fetchLine(true);

   if (srt->latestLine!=nullptr)
      delete srt->latestLine;

   if (line!=nullptr)
   {
      uint64_t   *p=rmlEval->wordBook-srt->spaceNode->varOffset;
      int         cc=csv->columnCount();

      for (int i=0;i<cc;i++)
      {
         string *t=line->getColumn(i);

         *((string **)p)=t;
         p--;
      }
   }
   srt->latestLine=line;

   return line!=nullptr;
}

uint64_t RMLEval::resetScanner(RMLEval *rmlEval, int64_t selectId)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];
   srt->startCSVProxy();

   return readLine(rmlEval, srt);
}

uint64_t RMLEval::advanceScanner(RMLEval *rmlEval, int64_t selectId)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];

   return readLine(rmlEval, srt);
}

int64_t RMLEval::aggregate(RMLEval *rmlEval, uint64_t selectId, uint64_t *values)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];
   int               cc=srt->selectStat->columnList->columnCount();
   ofstream         *os=srt->os;
   RMLSelectNode    *selectStat=srt->getSelectStat();

   values+=cc-1;
   for (int i=0;i<cc;i++)
   {
      RMLEvalAggregator *aggregator=&selectStat->aggregators[i];
      double d=*(double *)values;

      if (aggregator->count==0)
      {
         aggregator->min=
         aggregator->max=d;
      }
      else
      {
         if (aggregator->min>d)
            aggregator->min=d;

         if (aggregator->max<d)
            aggregator->max=d;
      }

      aggregator->count++;
      aggregator->sx+=d;
      aggregator->sx2+=d*d;

      values--;
   }

   return cc;
}

int64_t RMLEval::snapshot(RMLEval *rmlEval, uint64_t selectId, uint64_t *values)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];
   RMLSelectNode    *selectStat=srt->getSelectStat();
   ofstream         *os=srt->os;
   int               cc=selectStat->columnList->columnCount();

   values+=cc-1;
   for (int i=0;i<cc;i++)
   {
      RMLColumnSpec *colSpec=selectStat->columnAt(i);

      if (i>0)
         *os << ',';

      switch (colSpec->type)
      {
         case RMLEvalType::RMLNumber:
            *os<<*(double *)values;
            break;
         case RMLEvalType::RMLString:
            *os<<*(string *)*values;
            break;
         case RMLEvalType::RMLBoolean:
            *os<<*(bool *)*values;
            break;
      }
      delete colSpec;
      values--;
   }
   *os << '\n';

   return cc;
}

void RMLEval::setWordBase(RMLEval *rmlEval, uint64_t baseAddress)
{
   rmlEval->wordBook=((uint64_t *)baseAddress)-1;
}

void RMLEval::createResultSet(RMLEval *rmlEval, uint64_t selectId)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];
   string            fName=string("result")+to_string((int)selectId)+".txt";
   ofstream         *os=new ofstream(fName);
   RMLColumnList    *columnList=srt->getSelectStat()->columnList;
   int               cc=(int)columnList->columnCount();
   RMLColumnDef     *colDef=columnList->getColumnAt(0);

   *os << "\xef\xbb\xbf" << *colDef->columnAlias;

   if (srt->selectStat!=nullptr)
   {
      vector<RMLEvalAggregator>
                       *aggregators=&srt->selectStat->aggregators;
      int               agc=aggregators->size();
      for (int i=0;i<agc;i++)
         (*aggregators)[i].reset();
   }

   for (int i=1;i<cc;i++)
   {
      colDef=columnList->getColumnAt(i);
      *os << "," << *colDef->columnAlias;
   }
   *os << '\n';
   srt->os=os;
}

uint64_t RMLEval::finalizeResultSet(RMLEval *rmlEval, uint64_t selectId)
{
   RMLSpaceRuntime  *srt=&rmlEval->ss[selectId];
   uint64_t          retVal=0;
   RMLSelectNode    *selectStat=srt->getSelectStat();
   int               ac=(int)selectStat->aggregators.size();

   if (ac>0)
   {
      for (int ai=0;ai<ac;ai++)
      {
         RMLEvalAggregator    aggregator=selectStat->aggregators[ai];

         if (ai>0)
            *srt->os << ',';
         *srt->os << aggregator.calculate();
      }
      *srt->os << "\n";
   }

   srt->os->close();
   delete srt->os;

   if (selectStat->simpleValueEvaluator)
   {
      int         row=0;
      CSVProxy   *cp=srt->startCSVProxy();
      CSVLine    *line,
                 *vLine=nullptr;
      string     *v=nullptr;
      double      d;

      for (line=cp->fetchLine(true);line!=nullptr && row<2;line=cp->fetchLine(true))
      {
         if (row==0)
         {
            vLine=line;
            v=line->getColumn(0);
            retVal=(uint64_t)v;
         }
         else
            delete line;
         row++;
      }

      if (row==1)
         switch (selectStat->type)
         {
            case RMLEvalType::RMLString:
               v=new string(*v);
               retVal=(uint64_t)v;
               rmlEval->registerStringObject(v);
               break;
            case RMLEvalType::RMLNumber:
               d=stod(*v);
               *((double *)&retVal)=d;
               break;
            case RMLEvalType::RMLBoolean:
               d=stod(*v);
               retVal=d!=0;
         }
      else
         rmlEval->setExceptionWord(new string(RML_EXCEPTION_QRCN1));

      if (vLine!=nullptr)
         delete vLine;

      //delete cp;
   }

   return retVal;
}

string *RMLEval::allocatestring(RMLEval *RMLEval, string *s1)
{
   string *retVal=new string(*s1);

   RMLEval->registerStringObject(retVal);

   return retVal;
}

string *RMLEval::catstring(RMLEval *RMLEval, string *s1, string *s2)
{
   vector<string *> stringVectorToClean=RMLEval->stringVectorToClean;
   string *retVal=new string((*s1)+(*s2));

   RMLEval->registerStringObject(retVal);

   return retVal;
}

string *RMLEval::number2str(RMLEval *RMLEval, double n)
{
   string *retVal=new string(to_string(n));

   RMLEval->registerStringObject(retVal);

   return retVal;
}

int64_t RMLEval::strcmp(string *s1, string *s2, int64_t test)
{
   int   r=0,
         v=s1->compare(*s2);

   switch (test)
   {
      case 0:
         r=v==0;
         break;
      case 1:
         r=v!=0;
         break;
      case 2:
         r=v>0;
         break;
      case 3:
         r=v<0;
         break;
      case 4:
         r=v>=0;
         break;
      case 5:
         r=v<=0;
         break;
   }

   return r;
}

int64_t RMLEval::postexecutecleanup(RMLEval *RMLEval)
{
   vector<string *> stringVectorToClean=RMLEval->stringVectorToClean;

   int cnt=stringVectorToClean.size();

   for (int i=0;i<cnt;i++)
      delete stringVectorToClean[i];

   /*
   vector<RMLEvalRowSet *> rowSetVectorToClean=RMLEval->rowSetVectorToClean;
   cnt=RMLEval->rowSetVectorToClean.size();

   for (int i=0;i<cnt;i++)
      delete rowSetVectorToClean[i];
   */
   return true;
}

int64_t RMLEval::checkexception(RMLEval *RMLEval)
{
   return (int64_t)RMLEval->exceptionWord;
}

RMLSpaceRuntime::RMLSpaceRuntime(RMLEvalSpaceNode *pSpaceNode)
{
   selectStat=nullptr;
   spaceNode=pSpaceNode;
}

RMLSpaceRuntime::RMLSpaceRuntime(RMLSelectNode *pSelectStat)
{
   selectStat=pSelectStat;
   spaceNode=nullptr;
}

RMLSpaceRuntime::~RMLSpaceRuntime()
{
   if ((selectStat!=nullptr || !spaceNode->isExternal) && cp!=nullptr)
      delete cp;

   if (latestLine!=nullptr)
      delete latestLine;
}

string RMLSpaceRuntime::getFileName()
{
   if (selectStat!=nullptr)
      return string("result")+to_string((int)selectStat->selectStatRuntimeIndex)+".txt";

   if (!spaceNode->isExternal)
      return string("result")+to_string((int)spaceNode->spaceIndex)+".txt";

   return string("");
}

CSVProxy *RMLSpaceRuntime::startCSVProxy()
{
   if (selectStat!=nullptr || !spaceNode->isExternal)
   {
      string fileName=getFileName();

      if (cp!=nullptr)
         delete cp;

      cp=new CSVProxy(&fileName);
   }
   else
      cp=((RMLEvalExternalSpaceNode *)spaceNode)->csv;

   cp->resetScanner();

   return cp;
}

RMLSelectNode *RMLSpaceRuntime::getSelectStat()
{
   if (selectStat!=nullptr)
      return selectStat;

   return (RMLSelectNode *)spaceNode->left;
}

RMLEvalAggregator::RMLEvalAggregator(int pAggregateId, RMLEval *pRmlEval, RMLEvalExpNode *pAggregatingNode)
{
   aggregateId=pAggregateId;
   rmlEval=pRmlEval;
   aggregatingNode=pAggregatingNode;
}

void RMLEvalAggregator::reset()
{
   max=
   min=
   sx=
   sx2=
   count=0;
}

void RMLEval::setExceptionWord(string *pew)
{
   if (exceptionWord==nullptr)
      exceptionWord=pew;
   else
      delete pew;
}

double RMLEvalAggregator::calculate()
{
   double retVal=0;

   switch (aggregateId)
   {
      case 0:
         // Standard deviation
         if (count>0)
         {
            double m = sx / count;
            double variance = sx2 / count - m * m;
            retVal=sqrt(variance);
         }
         break;
      case 1:
         // Mean
         if (count>0)
            retVal=sx / count;
         else
            rmlEval->setExceptionWord(new string(RML_EXCEPTION_AGRCNT));
         break;
      case 2:
         // Count
         retVal=count;
         break;
      case 3:
         // Min
         retVal=min;
         break;
      case 4:
         // Max
         retVal=max;
         break;
      case 5:
         retVal=sx;

   }
   return retVal;
}

#if defined(DO_X64)

RMLDynamicFuncDesc::RMLDynamicFuncDesc(RMLDynamicFunc *pf, int pCodeAllocationSize)
{
   f=pf;
   codeAllocationSize=pCodeAllocationSize;
}

RMLDynamicFuncDesc::~RMLDynamicFuncDesc()
{
   if (f!=nullptr)
   {
      mprotect((void *)f, codeAllocationSize, PROT_READ | PROT_WRITE);
      free((void *)f);
   }
}

#endif

#endif
