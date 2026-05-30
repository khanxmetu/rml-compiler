
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

// RMLIC definitions
void RMLIC::writeAsJSON(ostream *outStream) {
}

// RMLEval definitions
RMLDynamicFunc* RMLEval::generateCode() {
}

void RMLEval::scanConstantFolding() {
}

void RMLEval::scanConstantFolding(RMLEvalExpNode *node) {
}

void RMLEval::scanForIC() {
}

void RMLEval::scanForIC(RMLEvalExpNode *node) {
}

void RMLEval::peepholeIC() {
}

double RMLEval::stddev(int selectNdx, int columnNdx) {
}

double RMLEval::mean(int selectNdx, int columnNdx) {
}

double RMLEval::count(int selectNdx, int columnNdx) {
}

double RMLEval::min(int selectNdx, int columnNdx) {
}

double RMLEval::max(int selectNdx, int columnNdx) {
}

double RMLEval::sum(int selectNdx, int columnNdx) {
}

double RMLEval::sin(double number) {
}

double RMLEval::cos(double number) {
}

double RMLEval::tan(double number) {
}

double RMLEval::pi() {
}

double RMLEval::atan(double number) {
}

double RMLEval::asin(double number) {
}

double RMLEval::acos(double number) {
}

double RMLEval::exp(double number) {
}

double RMLEval::ln(RMLEval *rmlEval, double number) {
}

double RMLEval::number(RMLEval *rmlEval, string *str) {
}

double RMLEval::random(double number) {
}

double RMLEval::print(string *str) {
}

double RMLEval::len(string *str) {
}

string * RMLEval::right(RMLEval *rmlEval, string *str, double n) {
}

string * RMLEval::left(RMLEval *rmlEval, string *str, double n) {
}

// RMLICInst definitions
void RMLICInst::writeAsJSON(ostream *outStream) {
}