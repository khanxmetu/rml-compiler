#ifndef __RMLSUP
#define __RMLSUP

#define _PROJECT02

#if !defined(PROJECT02)
#define _PROJECT03

#if !defined(PROJECT03)
#define PROJECT04

#if defined(PROJECT04)
#define DO_X64
#endif
#endif

#endif

#include <cstdint>
#include "csvlib.h"

// Macro definitions for the operator and instruction codes.
// Some of the instruction codes are for future use.
#define OP_NOP    0
#define OP_COMMA  1
#define OP_ASSIGN 2
#define OP_COND   3
#define OP_ALT    4
#define OP_BAND   5
#define OP_BOR    6
#define OP_EQ     7
#define OP_NEQ    8
#define OP_LT     9
#define OP_LTE    10
#define OP_GT     11
#define OP_GTE    12
#define OP_ADD    13
#define OP_SUB    14
#define OP_MUL    15
#define OP_DIV    16
#define OP_MINUS  17
#define OP_NOT    18
#define OP_SSPACE 19
#define OP_SELECT 20
#define OP_CALL   21
#define OP_CROSSP 22
#define LAST_OP   (OP_CROSSP)
#define JMP       (LAST_OP+1)
#define JF        (LAST_OP+2)
#define JT        (LAST_OP+3)
#define SYMREF    (LAST_OP+4)
#define CONST     (LAST_OP+5)
#define LRT       (LAST_OP+6)
#define POP       (LAST_OP+7)

#define INSID     (LAST_OP+8)
#define INSMEM    (LAST_OP+9)
#define INSFREF   (LAST_OP+10)

#define OPTIMIZE_DC_NONSELECT    1  // Dead code elimination on ineffective where condition - where is always false statically.
#define OPTIMIZE_DC_EXPPART      2  // Dead code elimination with expression parts having no effect.
#define OPTIMIZE_CM_DYNAFIX      4  // Ivariant code motion when dynamically fixed value expression detected.
#define OPTIMIZE_PH_TRIVIALJUMP  8  // Peephole optimization to eliminate conditional jump check - where is always true statically.

#define OPTIMIZE_ALL (OPTIMIZE_DC_NONSELECT | OPTIMIZE_DC_EXPPART | OPTIMIZE_CM_DYNAFIX | OPTIMIZE_PH_TRIVIALJUMP)

#define LRT_RESETSCANNER         0
#define LRT_ADVANCESCANNER       1
#define LRT_AGGREGATE            2
#define LRT_SNAPSHOT             3
#define LRT_SETWORDBASE          4
#define LRT_CREATERESULTSET      5
#define LRT_FINALIZERESULTSET    6
#define LRT_NUMBERTOSTR          7
#define LRT_STRCMP               8
#define LRT_CATSTRING            9
#define LRT_ALLOCATESTRING       10
#define LRT_POSTEXECUTECLEANUP   11
#define LRT_CHECKEXCEPTION       12

#define RML_EXCEPTION_QRCN1      "Subquery must return single row."
#define RML_EXCEPTION_AGRCNT     "Aggregate requires at least one row."
#define RML_EXCEPTION_STR2NUM    "Number conversion exception."
#define RML_EXCEPTION_MATHARG    "Invalid math argument."

// A few forward declarations. Thanks C++! You have to be single pass.
class MyParser;
class RMLNode;
class RMLSelectNode;
class RMLEvalExpNode;
class RMLIdentifierList;
class RMLFuncDesc;
class RMLMsgDesc;
class RMLMsgSet;
class RMLSymbolTable;
class RMLSymbolColumn;
class RMLSymbolSpace;
class RMLEval;
class RMLIC;
class RMLICInst;
class RMLCodeLocation;
class RMLEvalRowSet;
class RMLEvalSpaceNode;
class RMLEvalAggregator;

// User message severity levels.
enum class RMLEvalMsgSeverity
{
   Error=0,
   Warning=1,
   Info=2
};

// Type encoding
enum class RMLEvalType
{
   RMLNone=0,

   RMLNumber=1,
   RMLString=2,
   RMLBoolean=3,
   RMLRowSet=4
};

// Type descriptor. This is good enough to describe RML types.
struct RMLTypeDesc
{
   // Basic type. DGArray must not exist here when finalized.
   RMLEvalType  type;
   // Space dimension. Zero for no array. Any positive number is space
   // dimension count.
   int         dim;
   bool operator == (RMLTypeDesc other);
   bool operator != (RMLTypeDesc other);
};

#if defined(PROJECT04)
class RMLICInst
{
   public:
      int            // The opCode of the instruction. By default, the op code is OP-NOP
                     opCode=OP_NOP;
      int            // The immediate parameter value stored. Number of parameters, number of pops
                     // and similar data are stored in this member.
                     p1=0,
                     // The x64 code location where the first CPU instruction resides in
                     // target.
                     codeOffset=0;
      RMLEvalType     // The type data for the instruction
                     type=RMLEvalType::RMLNone;

      // Additional data associated with the instruction. Immediate values,
      // function indexes are stored here
      union
      {
         string        *strConstant=nullptr;
         double         numConstant;
         int            intConstant;
         bool           boolConstant;
      };

      RMLICInst(); // Default constructor requirement inherent from code location vector. Not used at all.
      RMLICInst(int pOpCode, int pP1);
      RMLICInst(int pOpCode, int pP1, RMLEvalType pType);
      virtual ~RMLICInst();

      virtual void writeAsJSON(ostream *outStream);
};

// Type alias for ease of coding. A code path is a vector of
// code locations.
typedef vector<int> RMLCodePath;

class RMLCodePathWindow
{
      void build(RMLIC *ic, int codeLocationIndex, int windowSize, int depth, int *builder);
   public:
      // The vector of the code paths.
      vector<RMLCodePath *>   path;

      RMLCodePathWindow();
      virtual ~RMLCodePathWindow();
      void build(RMLIC *ic, int codeLocationIndex, int windowSize);
};

class RMLCodeLocation
{
      // Calculates the predecessors.
      // The accumulator vector receives index of predecessor code location
      // for the possible flows.
      void           predecessors(vector<int> *accumulator);

      // Checks whether the specified code location index exists in the specified vector.
      bool           ivHasNdx(vector<int> *v, int ndx);

      // Appends the specified code location index to the specified vector preventing duplicates.
      void           ivAppend(vector<int> *v, int ndx);

      // Appends the specified code location vector to the specified vector preventing duplicates.
      void           ivAppend(vector<int> *v, vector<int> *s);
   public:
      RMLICInst      // The resident instruction.
                     inst;
      RMLIC          // Pointer to the owning IC
                    *ic=nullptr;

      // Relocation offset value for code motion when applied.
      // IC level peephole optimization moves the linear code to shrink its size.
      int            relocation=0;
      vector<int>    // The vector of incoming jumps
                    *ij=nullptr;

      RMLCodeLocation(); // This is the automatic constructor only for IC resizing required. Not used at all!
      RMLCodeLocation(RMLIC *pIc, RMLICInst i);
      ~RMLCodeLocation();

      // Registers a code location as an immediate flow-predecessor
      void registerPredecessor(int source);

      // Calculates the predecessors by avoiding code location revisits.
      // The accumulator vector receives index of predecessor code location
      // for the possible flows.
      vector<int> *predecessors();
};

// The class that represents the linear IR (Intermediate code)
class RMLIC
{
   public:
      // The main storage of the instructions.
      vector<RMLCodeLocation> code=vector<RMLCodeLocation>();

      RMLIC();
      virtual ~RMLIC();

      // Emits an instruction with the specified opCode and parameter
      RMLICInst *emitIC(int opCode, int p1);
      // Emits an instruction with the specified opCode, parameter, and type
      RMLICInst *emitIC(int opCode, int p1, RMLEvalType type);

      // Retrieves the instruction at the specified code location.
      RMLICInst *instructionAt(int ndx);

      // Retrieves the code location at the specified code location.
      RMLCodeLocation *codeLocationAt(int ndx);

      // Marks the instruction at the specified code location for removal
      bool markRemoval(int ndx, int count);

      // Removes all the instructions marked for removal by adjusting
      // jump targets and predecessors
      int applyRemoval();

      int instCount();
      int codeLocationIndex(RMLCodeLocation *cl);

      // Generates immediate jump vectors for each code location.
      void collectImmediateJumps();

      virtual void writeAsJSON(ostream *outStream);
      void writeReadable(ostream *outStream);
};
#endif

class RMLMsgSet
{
      // The vector of the message descriptor.
      vector<RMLMsgDesc *> messages;
   public:
      RMLMsgSet();
      virtual ~RMLMsgSet();

      static bool compareMessageLines(RMLMsgDesc *p1, RMLMsgDesc *p2);
      void appendMessage(int pLineNumber, string pMsg, RMLEvalMsgSeverity pS);
      void sort();
      int messageCount(RMLEvalMsgSeverity type);

      // Generates a JSON value, which is array of the messages contained.
      void writeAsJSON(ostream *os);

      // Reports to the standard output
      void writeToConsole();
};

class RMLProductSpace
{
   public:
      vector<RMLEvalRowSet>   spaceElement;

      RMLProductSpace();
      ~RMLProductSpace();
};

class RMLEvalRowSet
{
   public:
      string         fileName;
      FILE          *file=nullptr;
      uint64_t       firstRecordPtr;
      vector<string> columnNames,
                     currentRow;

      RMLEvalRowSet(string pFileName);
      virtual ~RMLEvalRowSet();

      string fetchNext();

      string columnValue(string columnName);
      int columnCount();

      virtual bool equalsToOther(RMLEvalRowSet *other)=0;
};

// A simple class to represent a symbol that describes a space
class RMLSymbolSpace
{
   public:
      string    // Space name. Must be null for the calculated spaces
               *spaceName,
                // Alias for the space
               *alias;
      int       // Space index identifies uniquely a space
                spaceIndex;

      RMLSymbolSpace(string *pSpaceName, string *pAlias, int spaceIndex);
      virtual ~RMLSymbolSpace();

};

// A simple class to represent a symbol that describes a column
class RMLSymbolColumn
{
   public:
      RMLSymbolSpace
                  // The space where this column belongs to
                 *space=nullptr;
      string      // The name of the symbol.
                 *name;
      RMLEvalType  // The type specifier for the symbol.
                  type;
      int         // Symbol index
                  ndx=-1;
#if defined(PROJECT04)
      int         // Symbol word index
                  wordIndex=-1;
#endif
      RMLSymbolColumn(RMLSymbolSpace *pSpace, string *pName, RMLEvalType pType);
      virtual ~RMLSymbolColumn();

      string fqn();
};

class RMLSymbolTable
{
      // The vector of the symbols in the table.
      // The symbol table was kept simple for ease of implementation.
      vector<RMLSymbolColumn *>   columns;
      vector<RMLSymbolSpace *>    spaces;
   public:
      RMLSymbolTable *parent=nullptr;

      RMLSymbolTable();
      virtual ~RMLSymbolTable();

      // Student assignment part

      int spaceCount();
      RMLSymbolSpace *spaceAt(int i);
      // Finds the space by using a string which can be either a table name or an alias.
      RMLSymbolSpace *findSpace(string *name);
      RMLSymbolSpace *findSpace(string *spaceName, string *alias);

      // Adds a apace to the symbol table
      RMLSymbolSpace *addSpace(string *spaceName, string *alias, int spaceIndex);

      // Finds the symbol specified by "space" and "name". Returns descriptor when found, nullptr otherwise.
      RMLSymbolColumn *findSymbol(RMLSymbolSpace *space, string *name);
      // Finds the symbol specified by "spaceName" and "name". Returns descriptor when found, nullptr otherwise.
      RMLSymbolColumn *findSymbol(string *spaceName, string *name);

      // Inserts the symbol specified by "spaceName" and "name".
      RMLSymbolColumn *addSymbol(string *spaceName, string *alias, string *name, RMLEvalType pType);

      RMLSymbolColumn *resolveSymbol(string *spaceName, string *name);

      // Retrieves the symbol with the given index
      RMLSymbolColumn *symbolAt(int ndx);
      // Returns the number of the symbols stored.
      int symbolCount();

      // Generates a JSON value, which is array of the symbols contained.
      void writeAsJSON(ostream *os);
};

#if defined(PROJECT04)

#if defined(DO_X64)

// Type alias for void function with no parameter.
// The x64 code generated by RML is of this type.
typedef void RMLDynamicFunc();

/*
 * The class to pack the x64 code generated. It keeps a pointer to the memory block
 * that contains the code and the size of the memory block together.
 * When deleted, it discards the memory block properly.
 */
class RMLDynamicFuncDesc
{
   public:
      RMLDynamicFunc    // The pointer to the code generated
                       *f;
      int               // The size of the memory block that keeps the code.
                        // This is integral multiple of process default page size.
                        // See aligned_malloc, mprotect documentation.
                        codeAllocationSize;

      RMLDynamicFuncDesc(RMLDynamicFunc *pf, int pCodeAllocationSize);
      ~RMLDynamicFuncDesc();
};
#endif

/*
 * The class to represent runtime properties of a select / space
 * Instances of this class carries these properties to runtime.
 * Ideally, the members should have been the properties required for runtime only.
 * This way, it would be possible to separate the code from the compile time constructs.
 * However, we prefer using compile time object references in this simple LP experiment.
 */
class RMLSpaceRuntime
{
   public:
      RMLSelectNode       *selectStat;
      RMLEvalSpaceNode    *spaceNode;
      ofstream            *os=nullptr;
      CSVProxy            *cp=nullptr;
      CSVLine             *latestLine=nullptr;

      RMLSpaceRuntime(RMLSelectNode *pSelectStat);
      RMLSpaceRuntime(RMLEvalSpaceNode *pSpaceNode);
      ~RMLSpaceRuntime();

      string               getFileName();
      CSVProxy            *startCSVProxy();
      RMLSelectNode       *getSelectStat();
};

class RMLEvalAggregator
{
   public:
      // Members for basic descriptive statistics (ANOVA)
      double   max=0,
               min=0,
               sx=0,
               sx2=0;
      int      count=0;
      // The aggregateId is the function index of the aggregate function.
      int      aggregateId=-1;
      RMLEval *rmlEval;
      RMLEvalExpNode
              *aggregatingNode;

      void reset();
      double calculate();
      RMLEvalAggregator(int pAggregateId, RMLEval *pRmlEval, RMLEvalExpNode *pAggregatingNode);
};

#endif

class RMLEval
{
   private:
      void populateSelectNodeSymbols(RMLSelectNode *node, RMLEvalSpaceNode *spaceNode);
   public:
      static string           // An array of operator describing strings that can be accessed by the op-codes.
                              opStr[],
                              // An array of type describing string for reporting. This can be accessed by the type enumeration "DGEvalType".
                              typeStr[],
                              // An array of severity describing string for user message reporting.
                              // This can be accessed by the type enumeration "DGEvalMsgSeverity".
                              severityStr[],
                              // An array of LRT Code explanations to assist readable instruction reporting
                              lrtNames[];
      static RMLFuncDesc      // The library structure, which is static for ease of implementation.
                              // Ideally, this is replaced by a model that supports dynamic library management.
                              lib[];
      static int              // This is the sorting vector for the "lib". The symbol resolution for the runtime library
                              // uses binary search that requires sorted entities.
                             *sVector;

      RMLEvalExpNode          // When root tree node is displaced (for optimizations or other reasons)
                              // this word is used to update the root of the expression tree.
                              // This is only required for traversals that mutate the tree.
                            **rootPointerWord=nullptr;
#if defined(PROJECT04)
      uint64_t                // Word book start address. The word book is the array of the words
                              // that holds the column values fetched from the spaces.
                             *wordBook=nullptr;
      vector<RMLSpaceRuntime>
                              // The vector of the spaces and select statements
                              // This vectors helps maps index values to select / space
                              // specific data.
                              ss;
#endif
      // Sorter comparison callback for the runtime library.
      // The library is sorted once.
      static int compareFDesc(const void *v1, const void *v2);

      vector<RMLSelectNode *> *stats;
      RMLEvalExpNode          // A method iterating over statements may set
                              // the following member to make the statement to the
                              // traversal methods.
                             *statementInScan;
      RMLSelectNode           // The member to store the encapsulating select operator node.
                              // A traversal method having OP_SELECT operator may update this member at the entering
                              // and restore this member just before exiting so that the child members
                              // can access the owning select node.
                             *scopingSelectNode=nullptr;
      RMLEvalSpaceNode        // The member to store the encapsulating select operator node.
                              // A traversal method having OP_SSPACE operator may update this member at the entering
                              // and restore this member just before exiting so that the child members
                              // can access the owning space node.
                             *processingSpace=nullptr;

      RMLMsgSet               // The set of the messages that will be part of the project output.
                              messageSet;
      MyParser                // The driver (parser) reference. It may come handy!
                             *parser;

      int                     // The requested optimizations. See the OPTIMIZATION_... constants.
                              optimization,
                              // Maximum number of words needed to complete execution.
                              maxSymbolDepth=0;
      RMLIC                   // Linear IR object that will be generated when no syntactic and
                              // semantic problems are found.
                             *ic=nullptr;
#if defined(DO_X64)
      RMLDynamicFuncDesc      // Generates x64 executable function using the IC
                             *generateCode();
#endif

      vector<string *>        // Accumulates the strings created during evaluations.
                              // Cleanup procedure (postexecutecleanup) frees the string objects.
                              stringVectorToClean;
      RMLEval(MyParser *pParser);
      virtual ~RMLEval();

      // Finds the descriptor of the function referred by the "fName". Returns nullptr if the function is not found.
      static RMLFuncDesc *findLibFunction(string fName);

      // Returns the descriptor of the function having the specified index.
      static RMLFuncDesc *libFunctionAt(int ndx);

      // Generates a vector of the subtrees under the left chain of a given operator.
      vector<RMLEvalExpNode *> *leftChainVector(RMLEvalExpNode *node, int opCode);

      // Generates a vector of the expression parts separated by comma operators.
      vector<RMLEvalExpNode *> *expressionPartVector(RMLEvalExpNode *node);

#if defined(PROJECT04)
      string                  // Nonzero means the execution was interrupted with runtime error.
                             *exceptionWord=nullptr;
      // Sets the exception word of RML execution
      void setExceptionWord(string *pew);
#endif

#if defined(PROJECT04)
      static uint64_t readLine(RMLEval *rmlEval, RMLSpaceRuntime *srt);

      // A node is discardable if it does not have ant select node and active forcing function.
      // This may help identify the nodes having no effect in computation.
      bool nodeDiscardable(RMLEvalExpNode *node);

      // 0 (LRT_RESETSCANNER): Reset scanner by space id.
      static uint64_t resetScanner(RMLEval *rmlEval, int64_t selectId);

      // 1 (LRT_ADVANCESCANNER): Advance scanner by space id.
      static uint64_t advanceScanner(RMLEval *rmlEval, int64_t selectId);

      // 2 (LRT_AGGREGATE): Aggregate columns by using select id.
      static int64_t aggregate(RMLEval *rmlEval, uint64_t selectId, uint64_t *values);

      // 3 (LRT_SNAPSHOT): Get column values by using select id.
      static int64_t snapshot(RMLEval *rmlEval, uint64_t selectId, uint64_t *values);

      // 4 (LRT_STOREROW): Store row by select id.
      //static uint64_t storeRow(RMLEval *rmlEval, uint64_t selectId);

      // 4 (LRT_SETWORDBASE): Informs the rmlEval about the base address of the temporary words.
      static void setWordBase(RMLEval *rmlEval, uint64_t baseAddress);

      // 5 (LRT_CREATERESULTSET): Create result set by select id.
      static void createResultSet(RMLEval *rmlEval, uint64_t selectId);

      // 6 (LRT_FINALIZERESULTSET): Finalize result set by select id.
      static uint64_t finalizeResultSet(RMLEval *rmlEval, uint64_t selectId);

      // 7 (LRT_NUMBERTOSTR): Converts a number to string.
      static string *number2str(RMLEval *rmlEval, double n);

      // 8 (LRT_STRCMP): Compares two strings an returns the test result as true or false.
      // test parameter encoding: 0->EQ, 1:NEQ, 2->GT, 3->LT, 4->GTE, 5->LTE
      static int64_t strcmp(string *s1, string *s2, int64_t test);

      // 9 (LRT_CATSTRING): Concatenates two strings and returns the concatenated string.
      static string *catstring(RMLEval *rmlEval, string *s1, string *s2);

      // 10 (LRT_ALLOCATESTRING): Allocates a string given a string constant detected in source.
      static string *allocatestring(RMLEval *rmlEval, string *s1);

      // 11 (LRT_POSTEXECUTECLEANUP): Performs clean-up after execution. This must be called before the final return instruction.
      static int64_t postexecutecleanup(RMLEval *rmlEval);

      // 12 (LRT_CHECKEXCEPTION): Checks for an exception and returns true if an exception record was generated.
      static int64_t checkexception(RMLEval *rmlEval);

      // LRT End
#endif

      // Student assignment part

      // Scans the product spaces to establish symbol tables.
      void scanCheckExtractSymbols();
      // This is the method to scan a RMLSelectNode
      // It is supposed to scan product space to populate the symbol table in the select node
      // and to perform additional semantic checks on the column list.
      void scanSelectNodeForSymbols(RMLSymbolTable *parent, RMLSelectNode *node);
      // This is the method to scan an ordinary expression node (a node not RMLSelect). It checks whether the node
      // is an ordinary node or not. If it is an ordinary node it simply scans the right and left children.
      // If not, it invokes the specialized method by type casting (scanSelectNodeForSymbols).
      void scanExpNodeForSymbols(RMLSymbolTable *parent, RMLEvalExpNode *node);

      // Scans the drafted statements to calculate types of the expressions.
      void scanCalculateTypes();
      // Scans the expression nodes to calculate types of the expressions.
      void scanCalculateTypes(RMLEvalExpNode *node);

#if defined(PROJECT04)
      // Scans the drafted statements to apply constant folding.
      void scanConstantFolding();
      // Scans the expression nodes to perform constant folding in expressions.
      void scanConstantFolding(RMLEvalExpNode *node);

      // Transforms a string constant opCode to the relevant LRT 3
      void transformStrConstToLRT(RMLEvalExpNode *node);

      // Scans the list of statements (root level select statement in stats) to generate intermediate code.
      void scanForIC();
      // Scans the expression nodes to generate intermediate code.
      void scanForIC(RMLEvalExpNode *node);

      // Scans the linear IR
      void peepholeIC();

      // Registers string objects for cleanup after execution
      void registerStringObject(string *str);
      // Registers string objects for cleanup after execution
      void registerRMLRowSetObject(RMLEvalRowSet *arr);
#endif

      // Destroys the given tree by applying proper traversal.
      RMLEvalExpNode *destroyExpTree(RMLEvalExpNode *node);

      // Produces a string constant to ease JSON generation.
      static void writeStrValue(ostream *os, string *strValue);

      // Generates a JSON value, which contains the whole result of language processing. See the samples.
      virtual void writeAsJSON(ostream *os);

      // LRT Start


      // Library Start

      static double stddev(int selectNdx, int columnNdx);
      static double mean(int selectNdx, int columnNdx);
      static double count(int selectNdx, int columnNdx);
      static double min(int selectNdx, int columnNdx);
      static double max(int selectNdx, int columnNdx);
      static double sum(int selectNdx, int columnNdx);

      static double sin(double number);
      static double cos(double number);
      static double tan(double number);
      static double pi();
      static double atan(double number);
      static double asin(double number);
      static double acos(double number);
      static double exp(double number);
      static double ln(RMLEval *rmlEval, double number);
      static double number(RMLEval *rmlEval, string *str);
      static double random(double number);
      static double print(string *str);
      static double len(string *str);
      static string *right(RMLEval *rmlEval, string *str, double n);
      static string *left(RMLEval *rmlEval, string *str, double n);

      // Library End
};

/*
 * An instance of this class binds a column alias to
 * an expression  node. The column list of a select operator
 * contains instance of this class.
 */
class RMLColumnDef
{
   public:
      // rootNode and the columnAlias are owned by this
      // instance.
      RMLEvalExpNode *rootNode;  // Non-transferrable ownership.
      string *columnAlias;       // Non-transferrable ownership.

      RMLColumnDef(RMLEvalExpNode *pRootNode, string *pColumnAlias);
      virtual ~RMLColumnDef();
};

/*
 * The class to represent the column list of a select operator.
 */
class RMLColumnList
{
   private:
      vector<RMLColumnDef *> columns;
   public:
      RMLColumnList(RMLColumnDef *pColumnDef);
      void addColumn(RMLColumnDef *pColumnDef);
      int columnCount();
      RMLColumnDef *getColumnAt(int ndx);
      ~RMLColumnList();
};

/*
 * This is the abstract base class for AST nodes. Each of the nodes in the AST is
 * either direct instance of this class or instance of a descendant.
 */
class RMLEvalNode
{
   public:
      int   // The number of the relevant line for this node.
            lineNumber=0;

      RMLEvalNode(int pLineNumber);
      virtual ~RMLEvalNode();

      // Generates a JSON value that conveys details of the node.
      // This must be overridden by concrete subclasses.
      virtual void writeAsJSON(ostream *os)=0;

      virtual const char *getNodeTypeStr()=0;
};

/*
 * This is the concrete base class for AST nodes. Each of the nodes in the AST is
 * either direct instance of this class or instance of a descendant.
 */
class RMLEvalExpNode : public RMLEvalNode
{
   public:
      RMLEvalExpNode  // The child node on the left. This is null for a leaf.
                    *left,
                     // The child node on the right. This is null for a leaf.
                    *right,
                     // The parent node
                    *parent=nullptr;
      int            // The op-code of the node. This is one of the op-code macros.
                     opCode;
      RMLEvalType    // The type of the code initialized as zero {type:none, dim:0};
                     type=RMLEvalType::RMLNone;

      vector<RMLEvalExpNode *>
                     // The parameter vector for the call operator
                    *pVector=nullptr;
      int            // The code location index where OP_ALT code for true ends
                     altTrueEnd=-1,
                     // The code location index where OP_ALT code for false ends
                     altFalseEnd=-2,
                     // This member holds LRT or RTL index values.
                     idNdx=-1,
                     // This member useful while checking accumulated comma operands.
                     stackLoad=1,
#if defined(PROJECT04)
                     // The number of the function calls found in this subtree.
                     // This helps identify the code fragments that are going to be eliminated.
                     functionCallCount=0,
                     // The number of non-idempotennt function calls in this subtree
                     nifc=0,
                     // The number of the symbol references in this subtree
                     symRefCount=0,
                     // Number of the select nodes found in this subtree
                     selectCount=0,
                     // Number of the activation forcing calls.
                     forceActiveCount=0,
#endif
                     // When true, IC instruction emitter will skip this node.
                     // This is for marking the nodes that will lead to dead code generation.
                     // This occurs when a sub-expression is coded with comma operators not accessing
                     // a function or not forming an array literal.
                     eliminateIC=false;

      // Union of the constants depending on the constant type.
      // stringValue is also used for the ids.
      // doubleValue is also used as call index value (call, crt)
      union
      {
         double      doubleValue;
         string     *stringValue=nullptr;
         bool        boolValue;
      };

      // Constructor in various tastes.
      RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *pLeft, int pPoCode);
      RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *pLeft, int pOpCode, RMLEvalExpNode *pRight);
      RMLEvalExpNode(int pLineNumber, double value, int pOpCode);
      RMLEvalExpNode(int pLineNumber, string *p1, RMLEvalExpNode *child, int pOpCode);
      RMLEvalExpNode(int pLineNumber, string *p1, int pOpCode);
      RMLEvalExpNode(int pLineNumber, bool value, int pOpCode);
      RMLEvalExpNode(int pLineNumber, RMLEvalExpNode *rowSetRoot);
      virtual ~RMLEvalExpNode();

      string symRefString();
      // Student assignment part
      // This method is the critical JSON node generator the node
      // This method generates a structure as a JSON syntax compliant text.çççççç
      virtual void writeAsJSON(ostream *os);
      // This method emits additional JSON members in the JSON node that is being
      // generated by the writeAsJSON method. The writeAsJSON method must call this
      // method before placing the terminating } character. This method comes in handy
      // for reporting the subclasses of this class.
      virtual void writeAdditionalMembers(ostream *os);

      // The method that returns the node type string. Each subclass of RMLEval,
      // must override this method to place a fixed node specific string.
      // See the sample JSON files for the possible values.
      virtual const char *getNodeTypeStr();
};

/*
 * This class is designed to represent a column that may be
 * retrieved by using the relevant RMLEvalSpaceNode methods (columnAt, findColumn).
 * See these methods for further information.
 */
class RMLColumnSpec
{
   public:
      // The name of the column
      string *name;

      // The type of the column
      RMLEvalType type;

      // The index of the column
      int columnNdx;

      RMLColumnSpec(string *pName, RMLEvalType pType, int pColumnNdx);
      ~RMLColumnSpec();
};

/*
 * The abstract class to represent a space in the AST.
 */
class RMLEvalSpaceNode : public RMLEvalExpNode
{
   public:
      // For space nodes, this indicates whether the space is external or not.
      bool isExternal=false;
      // Space index
      int spaceIndex=-1;

#if defined(PROJECT04)
      // The variable (symbol) offset of the firstColumn.
      int varOffset=-1;
#endif

      RMLEvalSpaceNode(int pLineNumber, string *pExternalSpaceName, RMLEvalExpNode *alias);
      RMLEvalSpaceNode(int pLineNumber, RMLSelectNode *pSelectNode, RMLEvalExpNode *alias);

      // Returns the space name as string pointer. The returned pointer should not be freed explicitly by the caller.
      string *getSpaceName();

      // Returns the alias name as string pointer. The returned pointer should not be freed explicitly by the caller.
      string *getSpaceAlias();

      // Generates and return a display name for the space.
      // This method helps proper reporting and debugging.
      string getSpaceDisplayName();

      // Returns the number of the columns found in the CSV file
      virtual int columnCount()=0;

      // Returns the column information found by the index number 'ndx'
      // The return value belongs to the caller, meaning that the caller becomes the owner
      // of the returned value and must delete it after using.
      virtual RMLColumnSpec *columnAt(int ndx)=0;

      // The method to locate the column specified by the columnId parameter.
      // The return value belongs to the caller, meaning that the caller becomes the owner
      // of the returned value and must delete it after using.
      virtual RMLColumnSpec *findColumn(string *columnId)=0;
};

/*
 * The class to represent external spaces in the anstract syntax tree.
 * The AST builder can instantiate an object in the processExternalSpace method.
 */
class RMLEvalExternalSpaceNode : public RMLEvalSpaceNode
{
   public:
      CSVProxy *csv=nullptr;

      // pExternalSpaceName is owned by this node. This is stored in stringValue of the node.
      RMLEvalExternalSpaceNode(int pLineNumber, string *pExternalSpaceName, RMLEvalExpNode *alias);
      virtual ~RMLEvalExternalSpaceNode();

      // Returns the number of the columns found in the CSV file
      virtual int columnCount();

      // Returns the column information found by the index number 'ndx'
      // The return value belongs to the caller, meaning that the caller becomes the owner
      // of the returned value and must delete it after using.
      virtual RMLColumnSpec *columnAt(int ndx);

      // The method to locate the column specified by the columnId parameter.
      // The return value belongs to the caller, meaning that the caller becomes the owner
      // of the returned value and must delete it after using.
      virtual RMLColumnSpec *findColumn(string *columnId);

      // The method that should be overridden to return the node type
      // This is required for proper reporting and may help debugging.
      virtual const char *getNodeTypeStr();
};

/*
 * The class to represent the AST nodes specific to calculated spaces.
 *
 * See the sibling class RMLEvalExternalSpaceNode and the base class for additional
 * remarks on members.
 */
class RMLEvalCalculatedSpaceNode : public RMLEvalSpaceNode
{
   public:
      RMLEvalCalculatedSpaceNode(int pLineNumber, RMLSelectNode *pSelectNode, RMLEvalExpNode *alias);

      virtual int columnCount();
      virtual RMLColumnSpec *columnAt(int ndx);
      virtual RMLColumnSpec *findColumn(string *columnId);
      virtual const char *getNodeTypeStr();
};

/*
 * The class to represent the AST node specific to a select operator.
 * A select operator is a complex compound having a column list, a product space
 * and a condition expression. AST traversal passes will be sensitive to
 * the instances of this class to implement many experiment specific details.
 */
class RMLSelectNode : public RMLEvalExpNode
{
   //private:
      //RMLColumnSpec *resolveSymbol(RMLEvalSpaceNode *spaceNode, string *spaceOrAlias, string *columnId);
   public :
      // The object that contains the list of the columns generated by this select operator.
      RMLColumnList    *columnList;

      // The symbol table of the select statement. The symbols in this table
      // are not generated by the column list, but by the elements (spaces) found
      // in the product space. This symbol table makes these symbols available to the
      /// expressions found in the column expressions (per each column) and the where condition.
      RMLSymbolTable    symbolTable;

      // Each select statement needs a set of words to represents the values accessible
      // through the symbols found in the symbolTable. The 80x86 code generation model
      // requires to assign an offset for each of the symbols in the activation of the
      // generated procedure. Each select operator, appends its symbols to the contguous block
      // of the words allocated by the encapsulating select node. So, the number of the words
      // inherited from the encapsulating parent select nodes becomes the base word index.
      // Any symbol will have a word index relative to the baseWordIndex member.
      int               baseWordIndex=0;

      // The index value for the row set created by this select operator
      // if this is a root level select statement.
      int               selectStatRuntimeIndex=-1;

      // When a select statement is expected to result in a simple value
      // (i.e. single row, single column)
      bool              simpleValueEvaluator=false;

#if defined(PROJECT04)
      // Aggregators are used when a select node has a column list of aggregate functions.
      vector<RMLEvalAggregator>
                        aggregators;
#endif

      RMLSelectNode(int pLineNumber, RMLColumnList *pColumnList,RMLEvalExpNode *pLeft, int pOpCode, RMLEvalExpNode *pRight);

      // The handy method for emitting member additional to the EMLEvalExpNode.
      // Column list is an example.
      virtual void writeAdditionalMembers(ostream *os);

      // Retrieves the column index of the column defined by the column list.
      // This is NOT a method to use while resolving symbol references from the encapsulated expression!
      // The symbolTable member must be used for symbol resolution purposes.
      int findColumnNdx(string *fieldId);

      // For a select node that creates a calculated node, the following member points to
      // OP_SSELECT node, which is an RMLEvalCalculatedSpace
      RMLEvalCalculatedSpaceNode *calculatingSpace=nullptr;

      // Returns the column defined in the column list section of the select operator.
      RMLColumnSpec *columnAt(int ndx);

      // This is NOT a method to use while resolving symbol references from the encapsulated expression!
      // The symbolTable member must be used for symbol resolution purposes.
      // RMLColumnSpec *resolveSymbol(string *spaceOrAlias, string *columnId);


      virtual ~RMLSelectNode();

      virtual const char *getNodeTypeStr();
};

// A descriptor class for a runtime library function.
// Currently, it lacks a pointer to the implementing code.
class RMLFuncDesc
{
   public:
      string      // The name of the function.
                  name;
      void        // The entry point of the function
                 *f;
      bool        // True if the function requires implicit pointer to the RMLEval instance.
                  // This is necessary for resource allocating functions to enable
                  // post execution clean-up.
                  // See RMLEval::registerStringObject(string *str)
                  // and RMLEval::registerDGArrayObject(DGEvalArray *arr)
                  requiresRMLEval,
                  // True if the function is an aggregate function
                  isAggregate,
                  // True if the function throws exception. This helps the code generator
                  // to identify the requirement for exception checker call.
                  throwsException,
                  // True if the function generates the same value v with given actual parameter set p
                  idempotent,
                  // True is the function makes an expression part active, affecting optimization
                  forceActive;
      int         // Number of the parameters for this function.
                  paraCount;
      RMLEvalType // Type descriptor array to describe the return type,
                  // and the parameter types. Item 0 is the return value.
                  // This is kept simple for ease of implementation.
                  // Ideally, this is variable based on the number of the parameters.
                  types[3];
};

// A simple descriptor for a user message.
class RMLMsgDesc
{
   public:
      int               // THe number of the relevant line. INT_MAX and INT_MIN specify
                        // that the line number will not be reported.
                        lineNumber;
      string            // The message text
                        msg;
      RMLEvalMsgSeverity   // Severity level of the message.
                        s;

      RMLMsgDesc(int pLineNumber, string pMsg, RMLEvalMsgSeverity pS);
};

#endif
