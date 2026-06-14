# RML Linear IR and Code Generation (Project 4)

## Build

```bash
# include path for flex via homebrew mac
INCLUDEFLAGS="-I/opt/homebrew/opt/flex/include"

bison -d rmlsyn.y
flex -+ rml.l
g++ -std=c++11 -o project4 rmlmain.cpp rmlsup.cpp studentpart.cpp studentpart2.cpp csvlib.cpp x64codegen.cpp MyParser.cpp MyFlexLexer.cpp rmlsyn.tab.cc lex.yy.cc $INCLUDEFLAGS

# run
./project4 <optional optimization parameter> <rml module file name>
```

## Code Generation
- X64 code generation is implemented 
- To inspect the dynamic function's memory, set a breakpoint at rmlmain.cpp:141 before execution. The variable `fd->f` points to the memory block of dynamically generated code.
- Alternatively the binary code can be dumped to a file and disassembled as demonstrated in code comment

### Miscellaneous
There was a minor bug I encountered and fixed in rmlsup.cpp:
- In `RMLEval::snapshot()` replaced:  
`*os<<*(bool *)*values;` with: `*os<<*(bool *)values;`