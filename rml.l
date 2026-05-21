%option noyywrap
%option yylineno
%{
using namespace std;
#include "rmlsyn.tab.hh"

#undef YY_DECL
#define YY_DECL int MyFlexLexer::lex(yy::MyParserBase::semantic_type *lval)

#include "MyFlexLexer.h"
#include "MyParser.h"


%}
digit          [0-9]
pdigit         [1-9]
positiveint    {pdigit}{digit}*
wp             {positiveint}|"0"
fp             ("0"*{pdigit}+)+
number         (({wp}("."{fp})?)|("."{fp}))(("E"|"e")("+"|"-")?{pdigit}{digit}*)?
idstarter      [A-Za-z_]
idchar         [A-Za-z_0-9]
hexdigit       [0-9a-fA-F]
strchar        [\x23-\xff]|[\x20-\x21]
charashex      "\\x"{hexdigit}{hexdigit}?
escaped        "\\n"|"\\r"|"\\"t|"\\\\"|"\\\""
str            \"({strchar}|{charashex}|{escaped})*\"
identifier     "$"?{idstarter}{idchar}*
whitesp        [ \n\t\r]*
%%
"select"       {return yy::MyParserBase::token::SELECT;}
"from"         {return yy::MyParserBase::token::FROM;}
"where"        {return yy::MyParserBase::token::WHERE;}
"as"           {return yy::MyParserBase::token::AS;}
"\+"		   {return yy::MyParserBase::token::PLUS;}
"\-"		   {return yy::MyParserBase::token::MINUS;}
"\*"		   {return yy::MyParserBase::token::MUL;}
"\/"		   {return yy::MyParserBase::token::DIV;}
"("            {return yy::MyParserBase::token::LP;}
")"		       {return yy::MyParserBase::token::RP;}
","            {return yy::MyParserBase::token::COMMA;}
"<"		       {return yy::MyParserBase::token::LT;}
"<="		   {return yy::MyParserBase::token::LTE;}
">"		       {return yy::MyParserBase::token::GT;}
">="		   {return yy::MyParserBase::token::GTE;}
"=="		   {return yy::MyParserBase::token::EQ;}
"!="		   {return yy::MyParserBase::token::NEQ;}
"&&"		   {return yy::MyParserBase::token::BAND;}
"||"		   {return yy::MyParserBase::token::BOR;}
"!"		       {return yy::MyParserBase::token::NOT;}
";"			   {return yy::MyParserBase::token::SEMICOLON;}
":"			   {return yy::MyParserBase::token::COLON;}
"?"			   {return yy::MyParserBase::token::QMARK;}
"wait"		   {return yy::MyParserBase::token::WAIT;}
"then"		   {return yy::MyParserBase::token::THEN;}
"true"         {return yy::MyParserBase::token::TRUE;}
"false"        {return yy::MyParserBase::token::FALSE;}
"\."           {return yy::MyParserBase::token::DOT;}
{number}       {return driver->getNumber();}
{identifier}   {return driver->getId();}
{str}          {return driver->getStr();}
{whitesp}         
.              {return yy::MyParserBase::token::YYUNDEF;}
%%