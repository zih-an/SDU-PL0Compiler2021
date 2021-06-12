#pragma once
#include "globalvars.h"
#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
using namespace std;

class SyntaxAls {
public:
	SyntaxAls(vector<LxclInfo>&);
	void run();					// 
	void layout();
	void LDR(Node*);
	string getSynRes(bool& err) { err = synErr; return _resStr; }
	vector<TgtLan> getSmnRes(bool& err) { err = smnErr; return code; }
	void TargetCodeOutput();

	Node* PROGRAM();			// 程序
	Node* SUBPROG(int px);		// 分程序
	Node* CONSTANTDECLARE();	// 常量说明部分
	Node* CONSTANTDEFINE();		// 常量定义
	Node* VARIABLEDECLARE();	// 变量说明部分
	Node* PROCEDUREDECLARE();	// 过程说明部分
	Node* PROCEDUREHEAD();		// 过程首部

	Node* SENTENCE();			// 语句
	Node* ASSIGNMENT();			// 赋值语句
	Node* COMBINED();			// 复合语句
	Node* IFSENTENCE();			// 条件语句
	Node* CALLSENTENCE();		// 过程调用语句
	Node* WHILESENTENCE();		// 当型循环语句
	Node* READSENTENCE();		// 读语句
	Node* WRITESENTENCE();		// 写语句
	Node* EMPTY();				// 空语句

	Node* CONDITION();			// 条件
	Node* EXPRESSION();			// 表达式
	Node* ITEM();				// 项
	Node* FACTOR();				// 因子


	// for test
	void outputTest();
	void PrintSymbolTable();
	void PrintTargetCode();

private:
	vector<LxclInfo> _wordseq;		// 词法分析结果
	int ptr;						// 索引当前词
	bool synErr;					// 词法分析错误

	Node *ASTroot;					// 语法分析树
	string _resStr;					// 语法分析结果字符串
	int cntPro;						// procedure嵌套计数

	SymbolTable symbolTbl;			// 符号表
	int dx;							// 在procedure和构造函数中 = 3;  and +1 when var declared
	bool smnErr;					// 语义错误

	vector<TgtLan> code;			// 目标代码
	int codeLine;					// 跟踪目标代码行号（==code.size()-1）
	void emit(int f, int l, int a) { code.emplace_back(f, l, a); }		// 写入目标代码
	
	map<int, string> mp_ins;
	vector<pair<int, SymbolNode> > incmpltCode;
};


/*8种目标代码：
* LIT: l域无效，将a放到栈顶
* LOD: 将于当前层层差为l的层，变量相对位置为a的变量复制到栈顶
* STO: 将栈顶内容复制到于当前层层差为l的层，变量相对位置为a的变量
* CAL: 调用过程。l标明层差，a表明目标程序地址
* INT: l域无效，在栈顶分配a个空间
* JMP: l域无效，无条件跳转到地址a执行
* JPC: l域无效，若栈顶对应的布尔值为假（即0）则跳转到地址a处执行，否则顺序执行
* OPR: l域无效，对栈顶和栈次顶执行运算，结果存放在次顶，a=0时为调用返回

//语句类型：
* <语句> → <赋值语句>|<条件语句>|<当型循环语句>|<过程调用语句>|<读语句>|<写语句>|<复合语句>|<空语句>
*/