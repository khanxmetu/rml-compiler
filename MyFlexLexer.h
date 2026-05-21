#ifndef MYFLEXLEXER_H_
#define MYFLEXLEXER_H_

class MyFlexLexer : public yyFlexLexer
{
   MyParser *driver;
public:
   MyFlexLexer(MyParser *pDriver);
   int lex(yy::MyParserBase::semantic_type *lval);
};

#endif
