#include <iostream>
#include <fstream>
#include "Interpreter.h"
using namespace std;


int main(void) {
	//fstream fs("G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/compiler/program.code");
	fstream fs("program.code");
	vector<TgtLan> code;
	TgtLan c;
	string func;
	map<string, int> mp_ins = {
		{"LIT", 1},
		{"LOD", 2},
		{"STO", 3},
		{"CAL", 4},
		{"INT", 5},
		{"JMP", 6},
		{"JPC", 7},
		{"OPR", 8}
	};
	while (fs >> func) {
		c.func = mp_ins[func];
		if (fs >> c.lyr >> c.amt)
			code.push_back(c);
		else break;
	}
	fs.close();

	//FILE *stream1, *stream2;
	//freopen_s(&stream1, "G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/spfa/test1.in", "r", stdin);
	//freopen_s(&stream1, "G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/ipin.txt", "r", stdin);
	//freopen_s(&stream2, "G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/ipout.txt", "w", stdout);

	Interpreter Itprt(code);
	Itprt.run();
	//fclose(stdin);
	//fclose(stdout);
	//system("pause");
	return 0;
}