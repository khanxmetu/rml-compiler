#ifndef _CSVLIB_H
#define _CSVLIB_H

enum class CSVCharType
{
   whiteSpace,
   eof,
   comma,
   dQuote,
   other,
   carriageReturn,
   newLine,
   invalid
};

class CSVLine
{
   private:
      vector<string *>  columns;
   public:
      CSVLine();
      virtual ~CSVLine();

      string *getColumn(int ndx);
      void addColumn(string *a);
      int columnCount();
};

class CSVProxy
{
   private:
      int lineCursor=0;
      string *fileName=nullptr;
      ifstream *inFile=nullptr;
      bool  streamOpen=false,
            ioError=false;
      int   dataPosition=-1;

      void skipBOM();

      // Line fetcher state and helper function(s)
      CSVLine *line=nullptr;
      int      wsPref=0;
      string  *columnStr=nullptr;
      void pushColumn();
      void resetColumn();
      void receiveColumns();
   public:
      CSVLine *descLine=nullptr;

      CSVProxy(string *fileName);
      virtual ~CSVProxy();

      int columnCount();
      string *columnName(int ndx);

      int findColumnNdx(string *columnName);
      CSVLine *fetchLine(bool verifyColumns);
      void resetScanner();
};

#endif
