
using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include "rmlsup.h"
#include "rmlsyn.tab.hh"
#include "MyParser.h"
#include "FlexLexer.h"
#include "MyFlexLexer.h"

#include "csvlib.h"

int main(int argc, char **argv)
{
   if (argc==2 || argc==3)
   {
      int   optimization=OPTIMIZE_ALL,
            fnIndex=1;
      bool  paraOK=true;

      if (argc==3)
      {
         string flag=string(argv[1]);

         paraOK=flag.length()>2 && flag.substr(0,2).compare("-p")==0;
         if (paraOK)
         {
            size_t s;
            string flagPara=flag.substr(2, flag.length()-2);

            optimization=stoi(flagPara, &s);
            paraOK=s==flagPara.length();
            if (paraOK)
            {
               paraOK=optimization>=0 && optimization<=OPTIMIZE_ALL;
               if (paraOK)
                  fnIndex++;
               else
                  cout << "Invalid optimization value after -p. It must be between 0 and 15." << endl;
            }
            else
               cout << "-p flag must be followed by a valid integer." << endl;
         }
         else
            cout << "Invalid optimization flag."  << endl;
      }

      if (paraOK)
      {
         string arg=string(argv[fnIndex]),
                inputFile=arg+".txt",
                outputFile=arg+".json",
                outputICFile=arg+"-IC.txt";
         bool   processed=false;

         ifstream is(inputFile);
         if (is.is_open())
         {
            MyParser *driver=new MyParser(optimization);
            yy::MyParserBase *base=new yy::MyParserBase(driver);
            RMLEval *rmlEval=new RMLEval(driver);

            ofstream os(outputFile);
            if (os.is_open())
            {
#if defined(PROJECT04)
               ofstream icos(outputICFile);

               if (icos.is_open())
               {
#endif
                  driver->compile(base, &is, rmlEval);

#if defined(PROJECT02)
                  ofstream  project2OutputFile=ofstream("project02.txt");
#endif
                  int pel=driver->getParseErrorLine();

                  if (pel>=0)
                  {
                     cout << "Not recognized!" << endl;
#if defined(PROJECT02)
                     project2OutputFile << "Line " <<  pel << ": Syntax Error" << endl;
#endif
                  }
                  else
                  {
                     cout << "Recognized!" << endl;
#if defined(PROJECT02)

                     project2OutputFile << driver->stats.size() << endl;
                     project2OutputFile << driver->selectOpCount << endl;
                     project2OutputFile << driver->spaceCount << endl;
                     project2OutputFile << driver->aliasCount << endl;
                     project2OutputFile << driver->numericConstantCount << endl;
                     project2OutputFile << driver->stringConstantCount << endl;
                     project2OutputFile << driver->variableReferenceCount << endl;
                     project2OutputFile << driver->fqVariableReferenceCount << endl;
                     project2OutputFile << driver->funcCallCount << endl;
#endif
                  }
#if defined(PROJECT02)
                  project2OutputFile.close();
#endif
#if defined(PROJECT04)
                  if (rmlEval->messageSet.messageCount(RMLEvalMsgSeverity::Error)==0)
                     rmlEval->ic->writeReadable(&icos);
#endif
                  processed=true;
#if defined(PROJECT04)
                  icos.close();
               }
               else
                  cout << "Unable to create the IC output file "<<outputICFile<<"."<<endl;
#endif
               rmlEval->writeAsJSON(&os);
               rmlEval->messageSet.writeToConsole();
               os.close();
            }
            else
               cout << "Unable to create the output file "<<outputFile<<"."<<endl;

            is.close();
#ifdef DO_X64
            if (processed && rmlEval->messageSet.messageCount(RMLEvalMsgSeverity::Error)==0)
            {
               RMLDynamicFunc *f=rmlEval->generateCode();
               if (f!=nullptr)
                  f();
               cout << "Function: " << (int *)f << endl;
            }
#endif
            if (rmlEval->exceptionWord!=nullptr)
               cout << "RML exception " << *rmlEval->exceptionWord << endl;
            delete rmlEval;
            delete driver;
         }
         else
            cout << "Unable to open the input file "<<inputFile<<"."<<endl;
      }
   }
   else
      cout << "Usage is "<<argv[0]<<" <optional optimization parameter> <dgeval module file name>";

   return 0;
}
