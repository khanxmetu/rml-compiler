using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <limits.h>

#include "rmlsyn.tab.hh"
#include "MyParser.h"
#include "FlexLexer.h"
#include "MyFlexLexer.h"

MyParser::MyParser(int pOptimization)
{
   optimization=pOptimization;
}

MyParser::~MyParser()
{
   if (lexer!=nullptr)
      delete lexer;

   if (base!=nullptr)
      delete base;

   int cnt=stats.size();

   for (int i=0;i<cnt;i++)
      delete stats[i];
}

int MyParser::lex(yy::MyParserBase::semantic_type *const lval)
{
   this->lval=lval;
   return lexer->lex(lval);
}

void MyParser::compile(yy::MyParserBase *pBase, ifstream *is, RMLEval *pRmlEval)
{
   base=pBase;
   rmlEval=pRmlEval;
   lexer=new MyFlexLexer(this);
   lexer->switch_streams(is);
   base->parse();

#if defined(PROJECT03) || defined(PROJECT04)
   rmlEval->scanCheckExtractSymbols();
   rmlEval->scanCalculateTypes();
   rmlEval->messageSet.sort();
#if defined(PROJECT04)
   if (rmlEval->messageSet.messageCount(RMLEvalMsgSeverity::Error)==0)
   {
	   rmlEval->scanConstantFolding();
      if (rmlEval->messageSet.messageCount(RMLEvalMsgSeverity::Error)==0)
      {
         rmlEval->scanForIC();
         if (optimization & (OPTIMIZE_PH_TRIVIALJUMP))
            rmlEval->peepholeIC();
      }
   }
#endif
#endif
   rmlEval->messageSet.appendMessage(INT_MAX, "Completed compilation.", RMLEvalMsgSeverity::Info);
}

int MyParser::getId()
{
   lval->STR=new string(lexer->YYText());

   return yy::MyParserBase::token::ID;
}

int MyParser::hexDigit(char c)
{
   if (c>='0' && c<='9')
      return c-'0';

   if (c>='A' && c<='F')
      return c-'A'+10;

   if (c>='a' && c<='f')
      return c-'a'+10;

   return -1;
}

string *MyParser::makeString(const char *rawStr)
{
   char *copyStr=strdup(rawStr),
        *p,
        *q;
   int   d;
   unsigned char c;

   q=copyStr;
   for (p=q+1;*p!='"';p++,q++)
   {
      switch (*p)
      {
         case '\\':
            p++;
            switch (*p)
            {
               case '\\':
                  *q='\\';
                  break;
               case 't':
                  *q='\t';
                  break;
               case 'r':
                  *q='\r';
                  break;
               case 'n':
                  *q='\n';
                  break;
               case '"':
                  *q='"';
                  break;
               case 'x':
                  p++;
                  c=(unsigned char)hexDigit(*p);
                  d=hexDigit(p[1]);
                  if (d>-1)
                  {
                     p++;
                     c=(c<<4) | (unsigned char)d;
                  }
                  *q=c;
            }
            break;
         default:
            *q=*p;

      }
   }
   *q=0;
   string *retVal=new string(copyStr);

   free(copyStr);

   return retVal;
}

int MyParser::getStr()
{
   lval->STR=makeString(lexer->YYText());
   return yy::MyParserBase::token::STR;
}

int MyParser::getNumber()
{
   lval->NUM=atof(lexer->YYText());
   return yy::MyParserBase::token::NUM;
}

void MyParser::setParseErrorLine()
{
   parseErrorLine=lexer->lineno();
   rmlEval->messageSet.appendMessage(parseErrorLine, "Syntax error", RMLEvalMsgSeverity::Error);
}

int MyParser::getParseErrorLine()
{
   return parseErrorLine;
}

void yy::MyParserBase::error(const std::string &msg)
{
   driver->setParseErrorLine();
}


