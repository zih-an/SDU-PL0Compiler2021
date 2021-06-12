#include "Interpreter.h"

Interpreter::Interpreter() {
	regAddr = 0;
	regT = 0;
	regB = 0;
	stksize = maxsize;
	stk = stkmx;
	mp_ins = {
		{1, "LIT"},
		{2, "LOD"},
		{3, "STO"},
		{4, "CAL"},
		{5, "INT"},
		{6, "JMP"},
		{7, "JPC"},
		{8, "OPR"}
	};
}
Interpreter::Interpreter(vector<TgtLan> &c) {
	code = c;
	regAddr = 0;
	regT = 0;
	regB = 0;
	stksize = maxsize;
	stk = stkmx;
	mp_ins = {
		{1, "LIT"},
		{2, "LOD"},
		{3, "STO"},
		{4, "CAL"},
		{5, "INT"},
		{6, "JMP"},
		{7, "JPC"},
		{8, "OPR"}
	};
}
// for test
void Interpreter::printStack() {
	printf("\n=========regI: %s\t%d\t%d\n", mp_ins[regI.func].c_str(), regI.lyr, regI.amt);
	for (int i = 0; i < regT; i++) {
		printf("%d\t%d\n", i, stk[i]);
	}
	printf("==================\n\n");
}



void Interpreter::run() {
	do {
		regI = code[regAddr++];
		//printStack();
		switch (regI.func)
		{
		case 1:
			LIT();
			break;
		case 2:
			LOD();
			break;
		case 3:
			STO();
			break;
		case 4:
			CAL();
			break;
		case 5:
			INT();
			break;
		case 6:
			JMP();
			break;
		case 7:
			JPC();
			break;
		case 8:
			OPR();
			break;
		}
	} while (regAddr != 0);
}


// �������ŵ���ջ����a��Ϊ����
void Interpreter::LIT() {
	stk[regT] = regI.amt;
	regT++;
}
// �������ŵ�ջ����a��Ϊ��������˵�����е����λ�ã�lΪ���ò���˵����Ĳ��ֵ
void Interpreter::LOD() {
	int var = stk[findPos(regI.lyr) + regI.amt];
	stk[regT] = var;
	regT++;
}
// ��ջ���������͵�ĳ������Ԫ�С�a,l��ĺ�����LOD����ͬ
void Interpreter::STO() {
	--regT;
	stk[findPos(regI.lyr) + regI.amt] = stk[regT];
}
// ���ù��̵�ָ�aΪ�����ù��̵�Ŀ���������е�ַ��lΪ���
void Interpreter::CAL() {
	/*
		SL����̬��������ָ����ù��̵�ֱ�����������ʱ���ݶεĻ���ַ��
		DL����̬��������ָ����øù���ǰ�������й��̵����ݶεĻ���ַ��
		RA�����ص�ַ����¼���øù���ʱĿ�����Ķϵ㣬����ʱ�ĳ����ַ�Ĵ���P��ֵ��
	*/
	stk[regT] = findPos(regI.lyr);
	stk[regT + 1] = regB;
	stk[regT + 2] = regAddr;
	regB = regT;
	regAddr = regI.amt;

}
// Ϊ�����õĹ��̣���������������ջ�п�����������a��Ϊ���ٵĸ���
void Interpreter::INT() {
	int size = regI.amt;
	regT += size;
}
// ������ת��ָ�aΪת���ַ
void Interpreter::JMP() {
	regAddr = regI.amt;
}
// ����ת��ָ���ջ���Ĳ���ֵΪ����ʱ��ת��a��ĵ�ַ������˳��ִ��
void Interpreter::JPC() {
	if (stk[--regT] == 0) 
		regAddr = regI.amt;
}
// ��ϵ���������㡣���������a��������������Ϊջ���ʹζ������ݽ������㣬�������ڴζ���a��Ϊ0ʱ���˳�������
void Interpreter::OPR() {
	int a, b;
	switch (regI.amt) {
	case 0:  // �˳�
		regT = regB;				// ջ��λ��ָ�����ڹ��̵Ļ�ַ����Ϊ��ջ����
		regAddr = stk[regT + 2];			// ��һ��ָ��ָ��p ָ��ǰ���̵� ���ص�ַ
		regB = stk[regT + 1];			// ��ַ ָ��ǰ���̵� ��̬������ʶ��λ��
		break;
	case 1:  // +
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = a + b;
		break;
	case 2:  // -
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = a - b;
		break;
	case 3:  // *
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = a * b;
		break;
	case 4:  // /
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = a / b;
		break;
	case 5:  // ==
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a == b);
		break;
	case 6:  // !=
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a != b);
		break;
	case 7:  // <
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a < b);
		break;
	case 8:  // <=
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a <= b);
		break;
	case 9:  // >
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a > b);
		break;
	case 10:  // >=
		regT--;
		a = stk[regT - 1], b = stk[regT];
		stk[regT - 1] = (a >= b);
		break;
	case 11:  // -��Ŀ
		stk[regT - 1] = -stk[regT - 1];
		break;
	case 12:  // odd
		stk[regT - 1] = (stk[regT - 1] % 2);
		break;
	case 13:  // read  cin
		cin >> stk[regT];
		regT++;
		break;
	case 14:  // write cout
		cout << stk[--regT] << endl;
		break;
	}
}


// ��̬�� ���Ҳ��Ϊlyr�Ļ�ַ
int Interpreter::findPos(int lyr) {
	int blink = regB;
	while (lyr--)
		blink = stk[blink];
	return blink;
}

