#!/bin/bash
g++ -c SymbolTable.cpp -o SymbolTable.o -O2
g++ -c LexicalAls.cpp -o LexicalAls.o -O2
g++ -c SyntaxAls.cpp -o SyntaxAls.o -O2
g++ -c maincom.cpp -o maincom.o -O2
g++ SymbolTable.o LexicalAls.o SyntaxAls.o maincom.o -o Compiler -lm

g++ -c Interpreter.cpp -o Interpreter.o -O2
g++ -c mainint.cpp -o mainint.o -O2
g++ Interpreter.o mainint.o -o Interpreter -lm