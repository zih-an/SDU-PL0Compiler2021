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

	Node* PROGRAM();			// ����
	Node* SUBPROG(int px);		// �ֳ���
	Node* CONSTANTDECLARE();	// ����˵������
	Node* CONSTANTDEFINE();		// ��������
	Node* VARIABLEDECLARE();	// ����˵������
	Node* PROCEDUREDECLARE();	// ����˵������
	Node* PROCEDUREHEAD();		// �����ײ�

	Node* SENTENCE();			// ���
	Node* ASSIGNMENT();			// ��ֵ���
	Node* COMBINED();			// �������
	Node* IFSENTENCE();			// �������
	Node* CALLSENTENCE();		// ���̵������
	Node* WHILESENTENCE();		// ����ѭ�����
	Node* READSENTENCE();		// �����
	Node* WRITESENTENCE();		// д���
	Node* EMPTY();				// �����

	Node* CONDITION();			// ����
	Node* EXPRESSION();			// ���ʽ
	Node* ITEM();				// ��
	Node* FACTOR();				// ����


	// for test
	void outputTest();
	void PrintSymbolTable();
	void PrintTargetCode();

private:
	vector<LxclInfo> _wordseq;		// �ʷ��������
	int ptr;						// ������ǰ��
	bool synErr;					// �ʷ���������

	Node *ASTroot;					// �﷨������
	string _resStr;					// �﷨��������ַ���
	int cntPro;						// procedureǶ�׼���

	SymbolTable symbolTbl;			// ���ű�
	int dx;							// ��procedure�͹��캯���� = 3;  and +1 when var declared
	bool smnErr;					// �������

	vector<TgtLan> code;			// Ŀ�����
	int codeLine;					// ����Ŀ������кţ�==code.size()-1��
	void emit(int f, int l, int a) { code.emplace_back(f, l, a); }		// д��Ŀ�����
	
	map<int, string> mp_ins;
	vector<pair<int, SymbolNode> > incmpltCode;
};


/*8��Ŀ����룺
* LIT: l����Ч����a�ŵ�ջ��
* LOD: ���ڵ�ǰ����Ϊl�Ĳ㣬�������λ��Ϊa�ı������Ƶ�ջ��
* STO: ��ջ�����ݸ��Ƶ��ڵ�ǰ����Ϊl�Ĳ㣬�������λ��Ϊa�ı���
* CAL: ���ù��̡�l������a����Ŀ������ַ
* INT: l����Ч����ջ������a���ռ�
* JMP: l����Ч����������ת����ַaִ��
* JPC: l����Ч����ջ����Ӧ�Ĳ���ֵΪ�٣���0������ת����ַa��ִ�У�����˳��ִ��
* OPR: l����Ч����ջ����ջ�ζ�ִ�����㣬�������ڴζ���a=0ʱΪ���÷���

//������ͣ�
* <���> �� <��ֵ���>|<�������>|<����ѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�����>
*/