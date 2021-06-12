#pragma once

#include "globalvars.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
using namespace std;

class LexicalAls {
public:
	LexicalAls();

	void run();						// begin(main)
	void restart();

	bool checkKeys();				// sym
	bool Identifier(char&);			// id
	bool Digit(char&);				// num
	bool checkOpts(char&);			// opt
	bool checkSeps(char&);			// sep

	void checkState();
	void output();
	void clear();

	vector<LxclInfo> getRes(bool &err) { err = lxcError; return outputs; }

private:
	string idntf, dgt, opt;
	int id_state, dgt_state, opt_state;		// state
	bool crct_id, crct_dgt, crct_opt;		// correctness
	bool lxcError;							// lexical error
	vector<LxclInfo> outputs;				// results

	const vector<string> keywords = { "CONST", "VAR", "PROCEDURE", "BEGIN", "END", "ODD", "IF", "THEN", "CALL", "WHILE", "DO", "READ", "WRITE" };
	const vector<string> operators = { "=", ":=", "+", "-", "*", "/", "#", "<", "<=", ">", ">=" };
	const vector<char> separators = { ';', ',', '.', '(', ')' };
	const vector<char> opts_st1 = { '=', ':', '+', '-', '*', '/', '#', '<', '>' };
};