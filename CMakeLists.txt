cmake_minimum_required(VERSION 3.16)
project(rml-parser CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# bison: generates rmlsyn.tab.cc and rmlsyn.tab.hh
BISON_TARGET(
        MyParser
        ${CMAKE_CURRENT_SOURCE_DIR}/rmlsyn.y
        ${CMAKE_CURRENT_BINARY_DIR}/rmlsyn.tab.cc
)

# flex: generates lex.yy.cc
FLEX_TARGET(
        MyScanner
        ${CMAKE_CURRENT_SOURCE_DIR}/rml.l
        ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc
        COMPILE_FLAGS "-+"
)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_executable(driver
        rmlmain.cpp
        rmlsup.cpp
        studentpart.cpp
        csvlib.cpp
        x64codegen.cpp
        MyParser.cpp
        MyFlexLexer.cpp
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS}
        studentpart2.cpp
)

target_include_directories(driver PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        /opt/homebrew/opt/flex/include
)

# CSV Files:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/students.csv
        ${CMAKE_CURRENT_BINARY_DIR}/students.csv
        COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/courses.csv
        ${CMAKE_CURRENT_BINARY_DIR}/courses.csv
        COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/grades.csv
        ${CMAKE_CURRENT_BINARY_DIR}/grades.csv
        COPYONLY)

# Test Files:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ic-samples/sample0.txt
        ${CMAKE_CURRENT_BINARY_DIR}/sample0.txt
        COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ic-samples/sample1.txt
        ${CMAKE_CURRENT_BINARY_DIR}/sample1.txt
        COPYONLY)