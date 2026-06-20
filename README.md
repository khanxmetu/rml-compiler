# RML Compiler

This project is a compiler for Rowset Manipulation Language (RML), an experimental SQL-like language designed to query and manipulate CSV files.  
The compiler performs lexical analysis, syntax analysis, semantic analysis, intermediate representation (IR) generation, multi-pass optimizations and native x64 code generation.  
Further details can be found in [assignment-docs](assignment-docs/).

## RML Language Overview
RML uses a ternary `select` operator to query data. While it relies on dynamic typing at runtime, it enforces static type checking during compilation.

**Primitive Types:**
* `number`: IEEE 754 double-precision floating-point values.
* `boolean`: Standard `true` or `false` literals.
* `string`: Character arrays.
* `rowset`: Encapsulated rows and columns, reducible to primitive types under specific conditions.

**Basic Syntax:**
```sql
select <column_list> from <product_space> where <condition>;
```
An RML program must contain at least one `select` statement.

---

## Compiler Design

1. **Frontend (Flex/Bison):** Parses the source code, checks for syntax and builds the initial abstract syntax tree composition.
2. **Semantic Analyzer:** Resolves variable scopes using hierarchical symbol tables and validates parameter types for language runtime functions.
3. **IR & IC Generation:** Translates expression trees into a stack-machine Intermediate Code (IC) format. Outputs include a JSON AST report and a human-readable linear IC text file.
4. **Native x64 Code Generation:** Translates the linear stack-machine IC directly into native x64 instructions. The compiler loads these instructions into an executable memory block and runs it. It also interfaces with RML runtime library to support operations like console output (print).
---

## Usage

```bash
cmake -S . -B build && cmake --build build
cd build

# copy input source file and csv files

# run
./project4 <optional optimization parameter> <rml module file name>
```

### Optimization Flags
Combine the following flags using bitwise addition to enable specific compiler optimizations. For example, `-p15` enables all optimizations, while `-p0` disables them completely.

| Flag | Optimization | Description |
| --- | --- | --- |
| -p1 | Dead Code Elimination | Skips scanner loops and aggregations if the `where` condition statically evaluates to `false`.|
| -p2 | Ineffective Expressions | Eliminates expression parts within comma operators that lack side-effects.|
| -p4 | Invariant Code Motion | Optimizes the loop execution template for dynamically calculatable idempotent statements.|
| -p8 | Peephole Optimization | Eliminates trivial conditional jumps immediately following a boolean constant push.|

## RML Code Examples
1. List the students who took the course code C05 and scored below 85
Present the course ids and names along with student's name name, and the score.
```sql
select C.coursename as cousename,
B.firstname + B.lastname as name,
B.sex=="F"?"Female":"Male" as sex,
A.grade as grade
from grades A, students B, courses C
where B.sid==A.sid && number(A.grade)<85;
```

2. Report the course codes, mean grades for the courses having the mean less than 85
```sql
select meangrades.cid as cid, meangrades.meangrade as meangrade from
(select C.cid as cid, (select mean(number(G1.grade)) as grade from grades G1 where G1.cid==C.cid) as meangrade from courses C where (select count() as cnt from grades G2 where G2.cid==C.cid)>0) meangrades
where number(meangrades.meangrade)<85;
```

3. Report the minimum success by mean observed in courses
```sql
select min(number(meangrades.meangrade)) as minmeangrade from
(select C.cid as cid, (select mean(number(G1.grade)) as grade from grades G1 where G1.cid==C.cid) as meangrade from courses C where (select count() as cnt from grades G2 where G2.cid==C.cid)>0) meangrades
where true;
```

4. Report the courses having the lowest mean grade
```sql
select meangrades.cid as cid, meangrades.coursename as coursename, meangrades.meangrade as meangrade from
(select C.cid as cid, C.coursename as coursename, (select mean(number(G1.grade)) as grade from grades G1 where G1.cid==C.cid) as meangrade from courses C where (select count() as cnt from grades G2 where G2.cid==C.cid)>0) meangrades
where number(meangrades.meangrade)==
(select min(number(meangrades2.meangrade)) as minmeangrade from
(select C.cid as cid, (select mean(number(G1.grade)) as grade from grades G1 where G1.cid==C.cid) as meangrade from courses C where (select count() as cnt from grades G2 where G2.cid==C.cid)>0) meangrades2
where true);
```

5. The list of the courses with no attendant
```sql
select C.cid as cid, C.coursename as coursename from courses C
where (select count() as cnt from grades G where G.cid==C.cid)==0;
```