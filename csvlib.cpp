using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include "csvlib.h"

CSVLine::CSVLine()
{
}

CSVLine::~CSVLine()
{
   for (int i=columns.size()-1;i>=0;i--)
      delete columns[i];
}

string *CSVLine::getColumn(int ndx)
{
   int s=(int)columns.size();
   if (ndx>=0 && ndx<s)
      return columns[ndx];

   return nullptr;
}

void CSVLine::addColumn(string *str)
{
   columns.push_back(str);
}

int CSVLine::columnCount()
{
   return columns.size();
}

CSVProxy::CSVProxy(string *fileName)
{
   this->fileName=new string(*fileName);
   inFile=new ifstream(*fileName,ios::binary);
   streamOpen=inFile->is_open();
   resetScanner();
}

void CSVProxy::receiveColumns()
{
   if (dataPosition==-1)
   {
      if (descLine!=nullptr)
         delete descLine;

      descLine=fetchLine(false);

      if (descLine!=nullptr)
      {
         int cnt=descLine->columnCount();

         if (cnt==0)
         {
            delete descLine;
            descLine=nullptr;
            ioError=true;
         }
         else
            dataPosition=inFile->tellg();
      }
   }
   else if (dataPosition>=0)
   {
      inFile->seekg(dataPosition);
   }
}

void CSVProxy::resetScanner()
{
   inFile->clear();
   inFile->seekg(0);
   skipBOM();
   ioError=false;
   receiveColumns();
}

void CSVProxy::skipBOM()
{
   if (streamOpen && !ioError)
   {
      char bomBuffer[4];

      inFile->read(bomBuffer, 4);
      int bc=inFile->gcount();

      if (bc>=4 && bomBuffer[0]=='\xff' && bomBuffer[1]=='\xfe' && bomBuffer[2]==0 && bomBuffer[3]==0)
         ;
      else if (bc>=3 && bomBuffer[0]=='\xef' && bomBuffer[1]=='\xbb' && bomBuffer[2]=='\xbf')
         inFile->seekg(3);
      else if (bc>=2 && ((bomBuffer[0]=='\xff' && bomBuffer[1]=='\xfe') || (bomBuffer[0]=='\xfe' && bomBuffer[1]=='\xff')))
         inFile->seekg(2);
      else
         inFile->seekg(0);
   }
}

void CSVProxy::resetColumn()
{
   columnStr=new string();
   wsPref=0;
}

void CSVProxy::pushColumn()
{
   line->addColumn(columnStr);
   resetColumn();
}

CSVLine *CSVProxy::fetchLine(bool verifyColumns)
{
   bool  error=verifyColumns && descLine==nullptr,
         accept=false;
   int   state=0,
         eofCount=0;
   char  c;

   if (!error)
   {
      line=new CSVLine();
      resetColumn();

      if (streamOpen && !ioError)
      {
         while (!error && !accept)
         {
            inFile->read(&c,1);

            if (inFile->good())
            {
               if (eofCount>0)
                  error=true;
               else
               {
                  CSVCharType ct;

                  if (c==26)
                     ct=CSVCharType::eof;
                  else if (c==',')
                     ct=CSVCharType::comma;
                  else if (c=='"')
                     ct=CSVCharType::dQuote;
                  else if (c=='\r')
                     ct=CSVCharType::carriageReturn;
                  else if (c=='\n')
                     ct=CSVCharType::newLine;
                  else if (c>0 && c<=32)
                     ct=CSVCharType::whiteSpace;
                  else if (c==0)
                     ct=CSVCharType::invalid;
                  else
                     ct=CSVCharType::other;

                  if (ct==CSVCharType::invalid)
                  {
                     error=true;
                  }
                  else if (ct==CSVCharType::eof)
                     eofCount++;
                  else
                     switch (state)
                     {
                        case 0: // Start
                           switch (ct)
                           {
                              case CSVCharType::carriageReturn:
                                 state=5;
                                 break;
                              case CSVCharType::newLine:
                                 state=6;
                                 break;
                              case CSVCharType::comma:
                                 pushColumn();
                                 break;
                              case CSVCharType::dQuote:
                                 error=wsPref==0;
                                 if (!error)
                                    state=2;
                                 break;
                              case CSVCharType::whiteSpace:
                                 wsPref++;
                              default:
                                 state=1;
                                 (*columnStr)+=c;
                           }
                           break;
                        case 1:
                           switch (ct)
                           {
                              case CSVCharType::comma:
                                 pushColumn();
                                 state=0;
                                 break;
                              case CSVCharType::carriageReturn:
                                 pushColumn();
                                 state=5;
                                 break;
                              case CSVCharType::newLine:
                                 pushColumn();
                                 accept=true;
                                 state=6;
                                 break;
                              default:
                                 (*columnStr)+=c;
                           }
                           break;
                        case 2:
                           switch (ct)
                           {
                              case CSVCharType::dQuote:
                                 state=3;
                                 break;
                              default:
                                 (*columnStr)+=c;
                           }
                           break;
                        case 3:
                           switch (ct)
                           {
                              case CSVCharType::dQuote:
                                 columnStr+=c;
                                 state=2;
                                 break;
                              case CSVCharType::whiteSpace:
                                 state=4;
                                 break;
                              case CSVCharType::comma:
                                 pushColumn();
                                 state=0;
                                 break;
                              default:
                                 error=true;
                           }
                           break;
                        case 4:
                           switch (ct)
                           {
                              case CSVCharType::whiteSpace:
                                 break;
                              case CSVCharType::comma:
                                 pushColumn();
                                 state=0;
                                 break;
                              case CSVCharType::carriageReturn:
                                 state=5;
                                 break;
                              case CSVCharType::newLine:
                                 state=6;
                                 accept=true;
                                 break;
                              default:
                                 error=true;
                           }
                           break;
                        case 5:
                           switch (ct)
                           {
                              case CSVCharType::newLine:
                                 state=6;
                                 accept=true;
                                 break;
                              default:
                                 error=true;
                           }
                           break;
                     }
               }
            }
            else
            {
               break;
            }
         }
      }

      if (!error && verifyColumns)
         error=line->columnCount() != descLine->columnCount();
   }

   if (error)
   {
      ioError=true;
      delete line;
      line=nullptr;
   }

   delete columnStr;

   return line;
}

int CSVProxy::findColumnNdx(string *fieldId)
{
   int retVal=-1;

   if (descLine!=nullptr)
   {
      int cnt=descLine->columnCount();

      for (int i=0;i<cnt;i++)
      {
         string *v=descLine->getColumn(i);

         if (v->compare(*fieldId)==0)
         {
            if (retVal<0)
               retVal=i;
            else
            {
               retVal=-1;
               break;
            }
         }
      }
   }

   return retVal;
}

int CSVProxy::columnCount()
{
   int retVal=-1;
   if (descLine!=nullptr)
      retVal=descLine->columnCount();

   return retVal;
}

string *CSVProxy::columnName(int ndx)
{
   string *retVal=nullptr;

   if (descLine!=nullptr && ndx>=0 && ndx<descLine->columnCount())
      retVal=descLine->getColumn(ndx);

   return retVal;
}

CSVProxy::~CSVProxy()
{
   if (descLine!=nullptr)
      delete descLine;

   if (fileName!=nullptr)
      delete fileName;

   if (inFile!=nullptr)
   {
      inFile->close();
      delete inFile;
   }
}
