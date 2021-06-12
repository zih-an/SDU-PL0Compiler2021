#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
using namespace std;


static const int maxsize = 1e8;
static int stkmx[maxsize];

struct TgtLan {
	int func, lyr, amt;
	TgtLan() {
		func = 0, lyr = 0, amt = 0;
	}
	TgtLan(int f, int l, int a) :func(f), lyr(l), amt(a) { }
};


class Interpreter {
public:
	Interpreter();
	Interpreter(vector<TgtLan> &c);
	void run();

	void LIT();
	void LOD();
	void STO();
	void CAL();
	void INT();
	void JMP();
	void JPC();
	void OPR();

	// for test
	void printStack();

private:
	vector<TgtLan> code;		// 目标代码
	//vector<int> stk;			// 解释执行栈数据区
	int *stk;
	int stksize;				// 栈大小
	int findPos(int lyr);		// 静态链 查找层差为lyr的基址

	map<int, string> mp_ins;

	// registers
	TgtLan regI;				// 指令寄存器
	int regAddr;				// 程序地址寄存器，指向下一条要执行的目标指令（相当于CODE数组的下标）
	int regT;					// 栈顶寄存器T，指出了当前栈中最新分配的单元（T也是数组S的下标）
	int regB;					// 基地址寄存器，指出每个过程被调用时，在数据区S中给出它分配的数据段起始地址，也称为基地址

	
};

