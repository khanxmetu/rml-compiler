// Student Part2: Implementation for Intermediate Code Gen, Optimizations and x64 Code Gen

#include <exception>
using namespace std;
// #include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "rmlsup.h"
#include <algorithm>
#include <cmath>
#include <ctime>

#include "rmlsyn.tab.hh"
#include "MyParser.h"
#include "FlexLexer.h"

//X64CodeBag definitions
void X64CodeBag::translateInstructionStudent(RMLICInst *inst){

// Machine code disassembly via:
// nasm -f elf64 scratch.asm -o scratch.o
// objdump -D scratch.o 

    int opCode = inst->opCode;
    int p=inst->p1;
    auto type = inst->type;

    switch (opCode) {
        case OP_BAND:
        case OP_BOR:
// pop rax
// and qword [rsp], rax
// or qword [rsp], rax
//    0:   58                      pop    %rax
//    1:   48 21 04 24             and    %rax,(%rsp)
//    5:   48 09 04 24             or     %rax,(%rsp)
            emitBytes(1, 0x58);
            emitBytes(4, 0x48, (opCode==OP_BAND ? 0x21: 0x09), 0x04, 0x24);
            break;
        case OP_NOT:
        // flip lsb only
// xor qword [rsp], 1
//    0:   48 83 34 24 01          xorq   $0x1,(%rsp)
            emitBytes(5, 0x48, 0x83, 0x34, 0x24, 0x01);
            break;
// Comparison instructions
//    0:   75 0a                   jne    c <l>
//    2:   74 08                   je     c <l>
//    4:   7d 06                   jge    c <l>
//    6:   7f 04                   jg     c <l>
//    8:   7e 02                   jle    c <l>
//    a:   7c 00                   jl     c <l>
// short jump codes for inverse of condition are expected in cb
        case OP_EQ:
            comparisonInstruction(type, 0x75);
            break;
        case OP_NEQ:
            comparisonInstruction(type, 0x74);
            break;
        case OP_LT:
            comparisonInstruction(type, 0x7d);
            break;
        case OP_LTE:
            comparisonInstruction(type, 0x7f);
            break;
        case OP_GT:
            comparisonInstruction(type, 0x7e);
            break;
        case OP_GTE:
            comparisonInstruction(type, 0x7c);
            break;
// Arithmatic instructions
//    0:   f2 0f 10 44 24 08       movsd  0x8(%rsp),%xmm0
//    6:   f2 0f 58 04 24          addsd  (%rsp),%xmm0
//    b:   48 83 c4 08             add    $0x8,%rsp
//    f:   f2 0f 11 04 24          movsd  %xmm0,(%rsp)

//   14:   f2 0f 59 04 24          mulsd  (%rsp),%xmm0
//   19:   f2 0f 5e 04 24          divsd  (%rsp),%xmm0
//   1e:   f2 0f 5c 04 24          subsd  (%rsp),%xmm0
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        {
            uint8_t code = opCode==OP_ADD ? 0x58 :
             opCode==OP_SUB ? 0x5c :
             opCode==OP_MUL ? 0x59 :
             0x5e;

            emitBytes(20,
                0xf2, 0x0f, 0x10, 0x44, 0x24, 0x08,
                0xf2, 0x0f, code, 0x04, 0x24,
                0x48, 0x83, 0xc4, 0x08,
                0xf2, 0x0f, 0x11, 0x04, 0x24
            );
        }
            break;
        case OP_MINUS:
//    0:   66 0f ef c0             pxor   %xmm0,%xmm0  sets xmm0=0 by xoring
//    4:   f2 0f 5c 04 24          subsd  (%rsp),%xmm0
//    9:   f2 0f 11 04 24          movsd  %xmm0,(%rsp)
            emitBytes(14,
                0x66, 0x0f, 0xef, 0xc0, 
                0xf2, 0x0f, 0x5c, 0x04, 0x24,
                0xf2, 0x0f, 0x11, 0x04, 0x24
            );
            break;
        case JMP:
        {
            int32_t offset = rmlEval->ic->instructionAt(p)->codeOffset - (codeLen+5);
            emitBytes(1, 0xe9); // jmp
            emitCodeFrag(offset); // rel32 offset
        }
            break;

        case JF:
        {
//   25:	58                   	pop    %rax
//   26:	48 85 c0             	test   %rax,%rax
//   29:	0f 84 fc ff ff ff    	je     2b <_other+0x6>
            int32_t offset = rmlEval->ic->instructionAt(p)->codeOffset - (codeLen+10);
            emitBytes(6,
                0x58,
                0x48, 0x85, 0xc0,
                0x0f, 0x84
            );
            emitCodeFrag((int32_t)offset); // rel32 offset
        }
            break;

        case JT:
        {
//   25:	58                   	pop    %rax
//   26:	48 85 c0             	test   %rax,%rax
//   29:	0f 85 fc ff ff ff    	jne    2b <_other+0x6>
            int32_t offset = rmlEval->ic->instructionAt(p)->codeOffset - (codeLen+10);
            emitBytes(6,
                0x58,
                0x48, 0x85, 0xc0,
                0x0f, 0x85
            );
            emitCodeFrag((int32_t)offset); // rel32 offset
        }
            break;

        case SYMREF:
        {
            // push value at address rbp - inst->p1 -1 onto stack

//    0:   ff b5 34 12 34 12       push   0x12341234(%rbp)
            int32_t offset = -(inst->p1+1)*8;
            emitBytes(2, 0xff, 0xb5); emitCodeFrag(offset);
        }
            break;
        case CONST:
            if(type==RMLEvalType::RMLBoolean) {
//    0:   6a 00                   push   $0x0
                emitBytes(2, 0x6a, (inst->boolConstant ? 0x01 : 0x00));
            }
            else if(type==RMLEvalType::RMLNumber) {
//    0:   48 b8 34 12 34 12 34    movabs $0x1234123412341234,%rax
//    7:   12 34 12 
//    a:   50                      push   %rax
                emitBytes(2, 0x48, 0xb8); emitCodeFrag(inst->numConstant);
                emitBytes(1, 0x50);
            }
            break;
        case POP:
        //   25:	58                   	pop    %rax
            emitBytes(1, 0x58);
            break;
    }
}

void X64CodeBag::backpatchInstruction(RMLICInst *inst) {
    if(inst->opCode == JF || inst->opCode == JT) {
        *(int32_t *)(codeBase+inst->codeOffset+6)=codeLen-(inst->codeOffset+10);
    }else if(inst->opCode == JMP) {
        *(int32_t *)(codeBase+inst->codeOffset+1)=codeLen-(inst->codeOffset+5);
    }
}

// RMLIC definitions
void RMLIC::writeAsJSON(ostream *outStream) {
    *outStream<<"[";
    for (int i=0; i<code.size(); i++) {
        code[i].inst.writeAsJSON(outStream);
        if (i!=code.size()-1) {
            *outStream<<", ";
        }
    }
    *outStream<<"]";
}

// RMLEval definitions
RMLDynamicFuncDesc* RMLEval::generateCode() {
    std::srand(std::time(nullptr));
    auto codeBag = X64CodeBag(this);
    codeBag.emitPrologue(maxSymbolDepth);

    ic->collectImmediateJumps();
    for (int i=0; i<ic->code.size(); i++) {
        auto &line = ic->code[i];
        if(line.ij!=nullptr) {
            for(auto ijIndex: *line.ij) {
                if(ijIndex<i) // if it was forward jump, update jump offset
                    codeBag.backpatchInstruction(ic->instructionAt(ijIndex));
            }
        }
        codeBag.translateInstruction(&line.inst);
        codeBag.translateInstructionStudent(&line.inst);
    }
    codeBag.emitEpilogue();
    return codeBag.createCodeBase();
    // return nullptr;
}

void RMLEval::scanConstantFolding() {
    for (auto stat: *stats) {
        scanConstantFolding(stat);
    }
}

void constFoldApplyOp(RMLEvalExpNode* left, RMLEvalExpNode* right, RMLEvalExpNode* node, RMLMsgSet& messageSet) {
    switch (node->opCode) {
    case OP_COMMA:
        break;
    case OP_COND:
        break;
    case OP_ALT:
        break;
    case OP_BAND:
        node->opCode = CONST;
        node->boolValue = left->boolValue && right->boolValue;
        break;
    case OP_BOR:
        node->opCode = CONST;
        node->boolValue = left->boolValue || right->boolValue;
        break;
    case OP_EQ:
        node->opCode = CONST;
        if (left->type == RMLEvalType::RMLBoolean)
            node->boolValue = left->boolValue == right->boolValue;
        else if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue == right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue == *right->stringValue;
        break;
    case OP_NEQ:
        node->opCode = CONST;
        if (left->type==RMLEvalType::RMLBoolean)
            node->boolValue = left->boolValue != right->boolValue;
        else if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue != right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue != *right->stringValue;
        break;
    case OP_LT:
        node->opCode = CONST;
        if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue < right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue < *right->stringValue;
        break;
    case OP_LTE:
        node->opCode = CONST;
        if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue <= right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue <= *right->stringValue;
        break;
    case OP_GT:
        node->opCode = CONST;
        if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue > right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue > *right->stringValue;
        break;
    case OP_GTE:
        node->opCode = CONST;
        if (left->type == RMLEvalType::RMLNumber)
            node->boolValue = left->doubleValue >= right->doubleValue;
        else if (left->type == RMLEvalType::RMLString)
            node->boolValue = *left->stringValue >= *right->stringValue;
        break;
    case OP_ADD:
        node->opCode = CONST;
        if (left->type==RMLEvalType::RMLNumber && right->type==RMLEvalType::RMLNumber)
        {
            node->doubleValue = left->doubleValue + right->doubleValue;
        }
        else if (left->type==RMLEvalType::RMLString && right->type==RMLEvalType::RMLString)
        {
            node->stringValue = new string(*left->stringValue + *right->stringValue);
        }
        else if ((left->type==RMLEvalType::RMLString && right->type==RMLEvalType::RMLNumber) || (left->type==RMLEvalType::RMLNumber && right->type==RMLEvalType::RMLString))
        {
            std::cout << "this should not be possible (as handled in scanCalculateTypes)" << std::endl;
        }
        break;
    case OP_SUB:
        node->opCode = CONST;
        node->doubleValue = left->doubleValue - right->doubleValue;
        break;
    case OP_MUL:
        node->opCode = CONST;
        node->doubleValue = left->doubleValue * right->doubleValue;
        break;
    case OP_DIV:
        if (right->doubleValue == 0) {
            node->doubleValue = 3;
            messageSet.appendMessage(node->lineNumber, "Division by zero is not allowed", RMLEvalMsgSeverity::Error);
        }else {
            node->opCode = CONST;
            node->doubleValue = left->doubleValue / right->doubleValue;
        }
        break;
    case OP_MINUS:
        node->opCode = CONST;
        node->doubleValue = -left->doubleValue;
        break;
    case OP_NOT:
        node->opCode = CONST;
        node->boolValue = !left->boolValue;
        break;
    case OP_SSPACE:
        break;
    case OP_SELECT:
        break;
    case OP_CALL:
        break;
    case OP_CROSSP:
        break;
    }

    // node was updated to const
    if (node->opCode == CONST) {
        node->left = nullptr;
        node->right = nullptr;
        delete left;
        delete right;
    }
}

void constFoldApplyIdentity(RMLEvalExpNode* konst, RMLEvalExpNode* other, RMLEvalExpNode* node, RMLEvalExpNode **rootPointerWord, RMLMsgSet& messageSet) {
    RMLEvalExpNode* escalatedNode = nullptr;
    switch (node->opCode) {
    case OP_ADD:
        if ((konst->type == RMLEvalType::RMLNumber && konst->doubleValue == 0) || (konst->type == RMLEvalType::RMLString && konst->stringValue!=nullptr && konst->stringValue->empty())) {
            escalatedNode = other;
        }
        break;
    case OP_MUL:
        if (konst->doubleValue == 0) {
            escalatedNode = konst;
        }else if(konst->doubleValue == 1) {
            escalatedNode = other;
        }
        break;
    case OP_SUB:
        if (konst->doubleValue == 0) {
            if (konst == node->right) {
                escalatedNode = other;
            }else {
                node->right = nullptr;
                node->opCode = OP_MINUS;
                node->left = other;
                delete konst;
            }
        }
        break;
    case OP_BAND:
        if (konst->boolValue) {
            escalatedNode = other;
        }else {
            escalatedNode = konst;
        }
        break;

    case OP_BOR:
        if (konst->boolValue) {
            escalatedNode = konst;
        }else {
            escalatedNode = other;
        }
        break;
    case OP_DIV:
        if (konst->doubleValue == 0) {
            if (konst == node->left) {
                escalatedNode = konst;
            }else {
                node->doubleValue = 3;
                messageSet.appendMessage(node->lineNumber, "Division by zero is not allowed", RMLEvalMsgSeverity::Error);
            }
        }else if (konst->doubleValue == 1 && konst == node->right) {
            escalatedNode = other;
        }
        break;
    case OP_COND:
        if (konst == node->left) {
            if (konst->boolValue) {
                escalatedNode = other->left;
            }else {
                escalatedNode = other->right;
            }
        }
        break;
    }

    if (escalatedNode!=nullptr) {
        // update parents child link
        if (node->parent != nullptr) {
            if (node->parent->left == node) {
                node->parent->left = escalatedNode;
            }
            if (node->parent->right == node) {
                node->parent->right = escalatedNode;
            }
        }else {
            // parent null means set it to root expr of column
            *rootPointerWord = escalatedNode;
            // can statement root be null?
        }
        // update parent link
        escalatedNode->parent = node->parent;

        delete (escalatedNode==konst ? other : konst);
    }
}

void mutateStrOpcode(RMLEvalExpNode* node) {
    if (node->left->type != RMLEvalType::RMLString || node->right->type != RMLEvalType::RMLString)
        return;
    switch (node->opCode) {
        case OP_EQ:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 0;
            break;
        case OP_NEQ:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 1;
            break;
        case OP_GT:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 2;
            break;
        case OP_LT:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 3;
            break;
        case OP_GTE:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 4;
            break;
        case OP_LTE:
            node->opCode = LRT;
            node->idNdx = LRT_STRCMP;
            node->doubleValue = 5;
            break;
        case OP_ADD:
            node->opCode = LRT;
            node->idNdx = LRT_CATSTRING;
            break;
    }
}

void RMLEval::scanConstantFolding(RMLEvalExpNode *node) {
    if (node==nullptr) return;
    RMLEvalExpNode *left = node->left, *right = node->right;

    RMLSelectNode *prevSelectNode = scopingSelectNode;


    if (node->opCode == OP_SELECT) {
        const auto selectNode = dynamic_cast<RMLSelectNode *>(node);
        scopingSelectNode = selectNode;
        // the baseIndex member is calculated as parent scopingSelectNode’s (the one saved in the previous article) baseIndex plus parent scopingSelectNode’s symbol count;
        if (prevSelectNode!=nullptr) {
            scopingSelectNode->baseWordIndex = prevSelectNode->baseWordIndex + prevSelectNode->symbolTable.symbolCount();
        }
        else scopingSelectNode->baseWordIndex = 0;
        // the number of the symbols defined by this node may be factored in to find the maximum number of the words (maxSymbolDepth) needed for the calculation. This is vital for generating x64 code.
        maxSymbolDepth = std::max(maxSymbolDepth, selectNode->baseWordIndex + selectNode->symbolTable.symbolCount());
    }

    scanConstantFolding(left);
    scanConstantFolding(right);
    left = node->left;
    right = node->right;

    // all params const
    if (left!=nullptr && left->opCode == CONST && (right==nullptr ||  right->opCode == CONST)) {
        constFoldApplyOp(left, right, node, messageSet);
    }
    // both params exist and at least one is non-const
    else if (left!=nullptr && right!=nullptr) {
        if (left->opCode == CONST) {
            constFoldApplyIdentity(left, right, node, rootPointerWord, messageSet);
        }else if (right->opCode == CONST) {
            constFoldApplyIdentity(right, left, node, rootPointerWord, messageSet);
        }

        // replace with lrt opcode for strings
        mutateStrOpcode(node);
    }

    // set stackLoad for comma tree
    if (node->opCode == OP_COMMA && (node->parent == nullptr || node->parent->opCode != OP_COMMA)) {
        // root of comma tree
        bool callContext = (node->parent != nullptr && node->parent->opCode == OP_CALL);
        bool optimize = (optimization & OPTIMIZE_DC_EXPPART);
        auto exprParts = leftChainVector(node, OP_COMMA);
        if(callContext || !optimize) node->stackLoad = exprParts->size();
        else {
            std::reverse(exprParts->begin(), exprParts->end());
            // i=0 always effective
            node->stackLoad = 1;
            for (int i=1; i<exprParts->size(); i++) {
                auto expr = exprParts->at(i);
                if (expr->forceActiveCount==0) {
                    expr->eliminateIC=true;
                    // expr->stackLoad=0;
                }else node->stackLoad++;
            }
        }
        delete exprParts;
    }
    

    if (node->opCode == OP_SELECT) {
        for (int i=0; i<scopingSelectNode->columnList->columnCount(); i++) {
            auto col = scopingSelectNode->columnList->getColumnAt(i);
            rootPointerWord = &col->rootNode;
            scanConstantFolding(col->rootNode);
        }
        // It computes the wordIndex of the symbols stored in the symbol table by scanning each symbol with help of symbolAt method. Note that this is an opportune time to combine the sym->ndx and this select node’s base word index.
        for (int i=0; i<scopingSelectNode->symbolTable.symbolCount(); i++) {
            auto sym = scopingSelectNode->symbolTable.symbolAt(i);
            sym->wordIndex = scopingSelectNode->baseWordIndex + sym->ndx;
        }
        scopingSelectNode=prevSelectNode;
    }
}

void RMLEval::scanForIC() {
    ic->emitIC(LRT, LRT_SETWORDBASE);
    for (auto stat: *stats) {
        scanForIC(stat);
        ic->emitIC(POP, stat->stackLoad);
    }
    ic->emitIC(LRT, LRT_POSTEXECUTECLEANUP);
}

void RMLEval::scanForIC(RMLEvalExpNode *node) {
    if (node==nullptr) return;
    if (node->eliminateIC) return;

    RMLEvalExpNode *left = node->left, *right = node->right;

    RMLSelectNode *prevSelectNode = scopingSelectNode;

    int selIndex = -1;
    bool dynafix=false;
    int whereSkipper=-1;

    //pre order
    if (node->opCode == OP_SELECT) {
        const auto selectNode = dynamic_cast<RMLSelectNode *>(node);
        scopingSelectNode = selectNode;
        selIndex = (selectNode->calculatingSpace!=nullptr) ? selectNode->calculatingSpace->spaceIndex : selectNode->selectStatRuntimeIndex;
        ic->emitIC(LRT, LRT_CREATERESULTSET)->intConstant = selIndex;

        // check staticSkip possible
        auto whereParts = leftChainVector(node->right, OP_COMMA);
        std::reverse(whereParts->begin(), whereParts->end());
        bool staticSkip = (optimization & OPTIMIZE_DC_NONSELECT) && (whereParts->at(0)->opCode==CONST && whereParts->at(0)->boolValue==false && right->selectCount==0);
        delete whereParts;

        if (staticSkip) {
            //finalize
            ic->emitIC(LRT, LRT_FINALIZERESULTSET)->intConstant=selIndex;
            scopingSelectNode=prevSelectNode;
            return;
        }
    }

    if (node->opCode!=OP_CALL)
        scanForIC(left);

    vector<int> resetSkippers;
    vector<RMLEvalExpNode *> *spaces = nullptr;

    // in order
    if (node->opCode==OP_SELECT) {
        spaces = leftChainVector(node->left, OP_CROSSP);
        reverse(spaces->begin(), spaces->end());
        int calcCount=0;
        int varOffset=scopingSelectNode->baseWordIndex;
        for (int i=0; i<spaces->size(); i++) {
            auto space = dynamic_cast<RMLEvalSpaceNode*>(spaces->at(i));
            if (!space->isExternal) calcCount++;
            space->varOffset=varOffset;
            varOffset+=space->columnCount();
        }
        if (calcCount > 0) {
            ic->emitIC(POP, calcCount);
        }
        // check dynafix possible
        dynafix=(OPTIMIZE_CM_DYNAFIX&optimization) && (right->selectCount==0 && right->nifc==0 && right->symRefCount==0 && right->forceActiveCount==0);
        if  (dynafix){
            scanForIC(right);
            whereSkipper=ic->instCount();
            ic->emitIC(JF, 0);
        }
        for (int i=0; i<spaces->size(); i++) {
            auto space = dynamic_cast<RMLEvalSpaceNode*>(spaces->at(i));
            ic->emitIC(LRT, LRT_RESETSCANNER)->intConstant = space->spaceIndex;
            resetSkippers.push_back(ic->instCount());
            ic->emitIC(JF, 0);
        }
        if (!dynafix) {
            scanForIC(right);
        }
    }
    else if (node->opCode==OP_COND) {
        // jf falseCase
        int jumpFalseIndex = ic->instCount();
        ic->emitIC(JF, 0, node->type);
        // trueCase:
        scanForIC(right->left);
        // jmp endofternary
        int jmpEndIndex = ic->instCount();
        ic->emitIC(JMP, 0, node->type);
        // falseCase:
        ic->instructionAt(jumpFalseIndex)->p1=ic->instCount();
        scanForIC(right->right);
        // end of ternary
        ic->instructionAt(jmpEndIndex)->p1=ic->instCount();
    }else {
        scanForIC(right);
    }

    //post order
    transformStrConstToLRT(node);

    switch (node->opCode) {
        case OP_COMMA:
            // root comma operator outside function call context
            if (node->parent == nullptr || (node->parent->opCode != OP_COMMA && node->parent->opCode != OP_CALL)) {
                if (node->stackLoad-1>0)
                    ic->emitIC(POP, node->stackLoad-1);
            }
            break;
        // case OP_COND:
        //     // backpatch
        //     break;
        case OP_ALT:
            break;
        case OP_EQ:
        case OP_NEQ:
        case OP_LT:
        case OP_LTE:
        case OP_GT:
        case OP_GTE:
            node->stackLoad = left->stackLoad + right->stackLoad -1;
            ic->emitIC(node->opCode, 0, left->type);
            break;
        case OP_BAND:
        case OP_BOR:
        case OP_NOT:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MINUS:
            node->stackLoad = left->stackLoad + (right!=nullptr?right->stackLoad:0) -1;
            ic->emitIC(node->opCode, 0, left->type);
            break;
        case CONST:
            if (node->type == RMLEvalType::RMLBoolean)
                ic->emitIC(node->opCode, 0, node->type)->boolConstant=node->boolValue;
            else if (node->type == RMLEvalType::RMLNumber)
                ic->emitIC(node->opCode, 0, node->type)->numConstant=node->doubleValue;
            break;
        case OP_CALL: {
            RMLFuncDesc  *func=RMLEval::lib+left->idNdx;
            auto params = leftChainVector(right, OP_COMMA);
            int pc = static_cast<int>(params->size());
            delete params;
            if (func->isAggregate) {
                if (pc==0)
                    ic->emitIC(CONST, 0, RMLEvalType::RMLNumber)->numConstant=1;
            }else {
                ic->emitIC(node->opCode, pc, node->type)->numConstant = left->idNdx;
            }
        }
            break;
        case SYMREF:
            {
            RMLSymbolColumn *column=scopingSelectNode->symbolTable.resolveSymbol(node->stringValue!=nullptr?node->stringValue:nullptr, node->left!=nullptr?node->left->stringValue:nullptr);
            ic->emitIC(SYMREF, column->wordIndex);
            }
            break;
        case LRT:
            {
            auto c = ic->emitIC(node->opCode, node->idNdx, node->type);
            if (node->idNdx == LRT_STRCMP) c->intConstant=node->doubleValue;
            else if (node->idNdx == LRT_ALLOCATESTRING) c->strConstant=node->stringValue;
            }
            break;
        case OP_SELECT:
            {
            if (!dynafix) {
                whereSkipper=ic->instCount();
                ic->emitIC(JF, 0);
            }

            // column computations
            for (int i=0; i<scopingSelectNode->columnList->columnCount(); i++) {
                auto col = scopingSelectNode->columnList->getColumnAt(i);
                scanForIC(col->rootNode);
            }
            bool isAgg= !scopingSelectNode->aggregators.empty();
            if (isAgg) {
                ic->emitIC(LRT, LRT_AGGREGATE)->intConstant = selIndex;
            }else {
                ic->emitIC(LRT, LRT_SNAPSHOT)->intConstant = selIndex;
            }
            if (!dynafix) {
                ic->instructionAt(whereSkipper)->p1=ic->instCount();
            }
            for (int i=(int) spaces->size()-1; i>=0; i--) {
                auto space = dynamic_cast<RMLEvalSpaceNode*>(spaces->at(i));
                ic->emitIC(LRT, LRT_ADVANCESCANNER)->intConstant = space->spaceIndex;
                // jt placed after corresponding resetSkipper/jf inst
                ic->emitIC(JT, resetSkippers[i]+1);
            }
            for (auto indx: resetSkippers) {
                // resetSkipper jf location patched to after loop ends
                ic->instructionAt(indx)->p1 = ic->instCount();
            }
            if (dynafix) {
                ic->instructionAt(whereSkipper)->p1=ic->instCount();
            }
            //finalize
            ic->emitIC(LRT, LRT_FINALIZERESULTSET)->intConstant=selIndex;
            scopingSelectNode=prevSelectNode;
            delete spaces;
            }
            break;
    }

}

void RMLEval::peepholeIC() {
    ic->collectImmediateJumps();
    for (int i=2; i<ic->code.size(); i++) {
        auto w = RMLCodePathWindow();
        w.build(ic, i, 2);
        auto cur = ic->instructionAt(i);

        bool onlyConstPredecessorsPossible = w.path.size()==1; // predecessor could only be const if pred not a jump
        auto predIndex = w.path[0]->at(0);
        auto pred = ic->instructionAt(predIndex);
        if (onlyConstPredecessorsPossible && pred->opCode == CONST && pred->type == RMLEvalType::RMLBoolean) {
            if ((pred->boolConstant==true && cur->opCode == JF )
                    || (pred->boolConstant==false && cur->opCode == JT)) {
                ic->markRemoval(predIndex, 2);
            }

            if ((pred->boolConstant==false && cur->opCode == JF )
                    || (pred->boolConstant==true && cur->opCode == JT)) {
                ic->markRemoval(predIndex, 1);
                cur->opCode = JMP;
            }
        }
        if (cur->opCode == JMP && cur->p1 == i+1) {
            ic->markRemoval(i, 1);
        }
    }
    ic->applyRemoval();
    // clean incoming jumps so it can be recalculated
    for(auto &line: ic->code)  {
        delete line.ij;
        line.ij=nullptr;
    }
}

// aggregates are handled via LRT_AGGREGATE call to RMLEvalAggregator::calculate()
// the following aggregates are dummy functions that are never called
double RMLEval::stddev(int selectNdx, int columnNdx) {
    return -1;
}

double RMLEval::mean(int selectNdx, int columnNdx) {
    return -1;
}

double RMLEval::count(int selectNdx, int columnNdx) {
    return -1;
}

double RMLEval::min(int selectNdx, int columnNdx) {
    return -1;
}

double RMLEval::max(int selectNdx, int columnNdx) {
    return -1;
}

double RMLEval::sum(int selectNdx, int columnNdx) {
    return -1;
}

// scalar functions
double RMLEval::sin(double number) {
    return std::sin(number);
}

double RMLEval::cos(double number) {
    return std::cos(number);
}

double RMLEval::tan(double number) {
    return std::tan(number);
}
double RMLEval::pi() {
    return 3.14159265358979323846264338327950288;
}

double RMLEval::atan(double number) {
    return std::atan(number);
}

double RMLEval::asin(double number) {
    return std::asin(number);
}

double RMLEval::acos(double number) {
    return std::acos(number);
}

double RMLEval::exp(double number) {
    return std::exp(number);
}

double RMLEval::ln(RMLEval *rmlEval, double number) {
    if(number<=0) {
        rmlEval->setExceptionWord(new string(RML_EXCEPTION_MATHARG));
        return -1;
    }else {
        return std::log(number);
    }
}

double RMLEval::number(RMLEval *rmlEval, string *str) {
    try{
        return std::stod(*str);
    } catch(std::exception& e) {
        rmlEval->setExceptionWord(new string(RML_EXCEPTION_STR2NUM));
        return -1;
    }
}

double RMLEval::random(double number) {
    return (float)(rand()) / RAND_MAX * number;
}

double RMLEval::print(string *str) {
    std::cout << str->c_str() << std::endl;
    return 1;
}

double RMLEval::len(string *str) {
    return str->size();
}

string * RMLEval::right(RMLEval *rmlEval, string *str, double n) {
    if(n<0 || n>str->size()) {
        rmlEval->setExceptionWord(new string(RML_EXCEPTION_MATHARG));
        return nullptr;
    }
    string* result = new string(str->substr(str->size()-n, (int)n));
    rmlEval->registerStringObject(result);
    return result;
}

string * RMLEval::left(RMLEval *rmlEval, string *str, double n) {
    if(n<0 || n>str->size()) {
        rmlEval->setExceptionWord(new string(RML_EXCEPTION_MATHARG));
        return nullptr;
    }
    string* result = new string(str->substr(0, (int)n));
    rmlEval->registerStringObject(result);
    return result;
}

// RMLICInst definitions
void RMLICInst::writeAsJSON(ostream *outStream) {
    *outStream <<"{"
        << "\"mnemonic\":"; RMLEval::writeStrValue(outStream, &RMLEval::opStr[opCode]);
    *outStream <<", " << "\"opCode\":" << opCode
        <<", " << "\"p1\":" << p1
        <<", " << "\"type\":" << static_cast<int>(type);

    if (opCode==CONST)
    {
        *outStream <<", " << "\"value\":";
        switch (type)
        {
            case RMLEvalType::RMLNumber:
                (*outStream) << numConstant;
                break;
            case RMLEvalType::RMLBoolean:
                (*outStream) << boolConstant;
                break;
            case RMLEvalType::RMLString:
                RMLEval::writeStrValue(outStream, strConstant);
                break;
            default: ;
        }
    }

    if (opCode==INSMEM)
    {
        if (strConstant!=nullptr) {
            *outStream <<", " << "\"value\":";
            RMLEval::writeStrValue(outStream, strConstant);
        }
    }

    if (opCode==LRT)
    {
        *outStream <<", " << "\"value\":";
        switch (p1)
        {
            case LRT_ALLOCATESTRING:
                RMLEval::writeStrValue(outStream, strConstant);
                break;
            case LRT_RESETSCANNER:
            case LRT_ADVANCESCANNER:
            case LRT_AGGREGATE:
            case LRT_SNAPSHOT:
            case LRT_CREATERESULTSET:
            case LRT_FINALIZERESULTSET:
            case LRT_STRCMP:
                (*outStream) << intConstant;
                break;
            default:
                *outStream << 0; // or null

        }

    }

    if (opCode==OP_CALL) {
        *outStream <<", " << "\"name\":"; RMLEval::writeStrValue(outStream, &RMLEval::libFunctionAt((int)numConstant)->name);
        *outStream <<", " << "\"id\":" << (int)numConstant;
    }

    *outStream << "}";
}