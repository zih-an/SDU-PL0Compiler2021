#pragma once
#include "globalvars.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>
using namespace std;


class SymbolTable {
public:
	SymbolTable(){
		mp = {
			{1, "CONSTANT"},
			{2, "VARIABLE"},
			{3, "PROCEDURE"}
		};
		valid = true;
	}
	bool insert(SymbolNode n, int lev);
	int checkPos(string name, vector<int> kind, int lev);
	SymbolNode getNode(int i) { return symbolTbl[i]; }
	int getSize() { return symbolTbl.size(); }
	bool setPara2(int pos, int val) {
		if (pos >= symbolTbl.size()) return false;
		symbolTbl[pos].para2 = val;
		return true;
	}

	// for test;
	void PrintSymbolTable();

private:
	map<int, string> mp;
	vector<SymbolNode> symbolTbl;	// ·ûºÅ±í
	bool valid;

};
