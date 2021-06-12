#pragma once
#include <vector>
#include <map>
#include <string>
using namespace std;

enum Type { SYM, ID, NUM, OPT, SEP, END };
struct LxclInfo {
	Type type;
	string val;
	LxclInfo() {}
	LxclInfo(Type t, string v) :type(t), val(v) {}
};

/* Syntax Tree */
struct Node {
	string val;
	vector<Node*> children;

	Node() {}
	Node(string v) :val(v) {}
	void InsertChild(Node* child) {
		children.push_back(child);
	}
};

/* Symbol Table info */
struct SymbolNode {
	string name, kind;
	int para1, para2;
	SymbolNode() {
		para1 = 0;
		para2 = 0;
	}
	SymbolNode(string n, string k, int p1, int p2) :name(n), kind(k), para1(p1), para2(p2) {}
	bool operator ==(SymbolNode &b) {
		if (this->name == b.name && this->kind == b.kind) return true;
		return false;
	}
};


/* Target Language 
func:
	+  -  *  /  =  #  <  <=  >  >=
	1  2  3  4  5  6  7   8  9  10
	11(单目运算- 参考表达式[+|-])
	12(条件语句odd判奇运算)
	13(读语句)
	14(写语句)
*/
struct TgtLan {
	int func, lyr, amt;
	TgtLan() { 
		func = 0, lyr = 0, amt = 0;
	}
	TgtLan(int f, int l, int a):func(f), lyr(l), amt(a) { }
};
