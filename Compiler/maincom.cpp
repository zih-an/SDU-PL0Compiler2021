#include "globalvars.h"
#include "LexicalAls.h"
#include "SyntaxAls.h"
using namespace std;

int main(void) {
	//FILE *stream1, *stream2;
	//freopen_s(&stream1, "G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/input.txt", "r", stdin);
	//freopen_s(&stream2, "G:/��ѧ����/������/����ԭ��/ʵ��ָ��/labs/output.txt", "w", stdout);
	//freopen_s(&stream2, "program.code", "w", stdout);
	freopen("program.code", "w", stdout);
	LexicalAls Lex;
	Lex.run();

	bool LexErr;
	vector<LxclInfo> wordseq = Lex.getRes(LexErr);
	if (LexErr) {
		return -1;
		//cout << "Lexical Error" << endl;
	}
	else {
		SyntaxAls Syn(wordseq);
		Syn.run();
		//Syn.PrintSymbolTable();
		//cout << endl << endl;
		//Syn.PrintTargetCode();
		Syn.TargetCodeOutput();
		cout << endl << endl;

		bool SynErr, SymErr;
		string synStr = Syn.getSynRes(SynErr);
		vector<TgtLan> code = Syn.getSmnRes(SymErr);
		if (SynErr) {
			return -1;
			//cout << "Syntax Error" << endl;
		}
		if (SymErr) {
			return -1;
			//cout << "Semantic Error" << endl;
		}
		else {
			//cout << synStr << endl;
			/* execute code here */

		}
	}
	//fclose(stdin);
	//fclose(stdout);

	//system("pause");
	return 0;
}
