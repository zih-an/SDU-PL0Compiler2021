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
	vector<TgtLan> code;		// Ŀ�����
	//vector<int> stk;			// ����ִ��ջ������
	int *stk;
	int stksize;				// ջ��С
	int findPos(int lyr);		// ��̬�� ���Ҳ��Ϊlyr�Ļ�ַ

	map<int, string> mp_ins;

	// registers
	TgtLan regI;				// ָ��Ĵ���
	int regAddr;				// �����ַ�Ĵ�����ָ����һ��Ҫִ�е�Ŀ��ָ��൱��CODE������±꣩
	int regT;					// ջ���Ĵ���T��ָ���˵�ǰջ�����·���ĵ�Ԫ��TҲ������S���±꣩
	int regB;					// ����ַ�Ĵ�����ָ��ÿ�����̱�����ʱ����������S�и�������������ݶ���ʼ��ַ��Ҳ��Ϊ����ַ

	
};

