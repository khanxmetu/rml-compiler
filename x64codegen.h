#ifndef _X64CODEGEN
#define _X64CODEGEN

#define DELTA 16

#define REG_0     0
#define REG_1     1
#define REG_2     2
#define REG_3     3
#define REG_4     4
#define REG_5     5
#define REG_6     6
#define REG_7     7

#define IREG_RAX   REG_0
#define IREG_RCX   REG_1
#define IREG_RDX   REG_2
#define IREG_RBX   REG_3
#define IREG_RSP   REG_4
#define IREG_RBP   REG_5
#define IREG_RSI   REG_6
#define IREG_RDI   REG_7

#define XMM_R0    0
#define XMM_R1    1
#define XMM_R2    2
#define XMM_R3    3
#define XMM_R4    4
#define XMM_R5    5
#define XMM_R6    6
#define XMM_R7    7
#define XMM_R8    8
#define XMM_R9    9
#define XMM_R10  10
#define XMM_R11  11
#define XMM_R12  12
#define XMM_R13  13
#define XMM_R14  14
#define XMM_R15  15

class X64CodeBag
{
   static int     regs[4];
   unsigned char *codeBase;
   int            bagSize,
                  codeLen,
                  unwindLocation=0;
   RMLEval       *rmlEval;
   vector<int>    unwindFixups;

   public:
      X64CodeBag(RMLEval *pRmlEval);
      virtual ~X64CodeBag();

      template <typename t> void emitCodeFrag(t codeFrag);
      void emitBytes(int len, ...);

      RMLDynamicFuncDesc* createCodeBase();

      void emitPrologue(int varCount);
      void emitEpilogue();
      void emitCall(void *callAddr);
      void emitExceptionChecker();
      void setupParameter(int ndx, bool isDouble);
      void setupImmediateIntegralPara(int ndx, uint64_t p);
      void setupImmediateDoublePara(int ndx, double p);
      void placeResultOnStack(bool isDouble);

      void translateInstructionStudent(RMLICInst *inst);
      void translateInstruction(RMLICInst *inst);
      void backpatchInstruction(RMLICInst *inst);
      void xmmArithInstruction(unsigned char cb);
      void comparisonInstruction(RMLEvalType td, unsigned char cb);
};

template <typename t> void X64CodeBag::emitCodeFrag(t codeFrag)
{
   int sz=(int)sizeof(codeFrag);
   if (codeLen+sz>=bagSize)
   {
      int newSize=bagSize+DELTA+sz;
      unsigned char *newBase=(unsigned char *)realloc(codeBase, newSize);

      if (newBase!=nullptr)
      {
         codeBase=newBase;
         bagSize=newSize;
      }
   }

   if (codeLen<bagSize)
   {
      *(t *)(codeBase+codeLen)=codeFrag;
      codeLen+=sz;
   }
};

#endif
