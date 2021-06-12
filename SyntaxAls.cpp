#include "SyntaxAls.h"
#include<cstdio>

SyntaxAls::SyntaxAls(vector<LxclInfo>& wordseq) {
	_wordseq = wordseq;
	_wordseq.emplace_back(END, "$");
	ptr = 0;
	synErr = false;
	smnErr = false;
	ASTroot = NULL;
	cntPro = 0;
	dx = 3;
	codeLine = 0;
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
void SyntaxAls::run() {
	Node* root = PROGRAM();
	if (!synErr) {
		layout();
	}
}
void SyntaxAls::layout() {
	//_resStr
	LDR(ASTroot);
}
void SyntaxAls::LDR(Node* node) {
	if (node != NULL) {
		_resStr += node->val;
		if (!node->children.empty()) {
			_resStr += "(";
			for (int i = 0; i < node->children.size(); i++) {
				LDR(node->children[i]);
				if (i != node->children.size() - 1)
					_resStr += ",";
			}
			_resStr += ")";
		}
	}
}
// for test
void SyntaxAls::outputTest() {
	cout << "	TEST in" << endl;
	cout << "type: " << _wordseq[ptr].type << "   val: " << _wordseq[ptr].val << endl;
	cout << "error: " << synErr << endl;
}
void SyntaxAls::PrintSymbolTable() {
	symbolTbl.PrintSymbolTable();
}
void SyntaxAls::PrintTargetCode() {
	printf("odr\tf\tl\ta\n");
	printf("===================================\n");
	int i = 0;
	for (TgtLan lan : code) {
		//printf("%d\t%s\t%d\t%d\n", i++, mp_ins[lan.func].c_str(), lan.lyr, lan.amt);
	}
}
void SyntaxAls::TargetCodeOutput() {
	for (TgtLan lan : code) {
		printf("%s %d %d\n", mp_ins[lan.func].c_str(), lan.lyr, lan.amt);
	}
}

/**
 * <����>��<�ֳ���>.
 */
Node* SyntaxAls::PROGRAM() {
	ASTroot = new Node("PROGRAM");
	ASTroot->InsertChild(SUBPROG(-1));
	if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ".") {
		ASTroot->InsertChild(new Node("."));
		++ptr;

		// 
		if (_wordseq[ptr].type == END && _wordseq[ptr].val == "$")
			return ASTroot;
	}
	synErr = true;
	return NULL;
}
/** ���������3��˵�����ֽ������ֱ���䲿������������
 * <�ֳ���>�� [<����˵������>][<����˵������>][<����˵������>]<���>
 */
Node* SyntaxAls::SUBPROG(int px) {	// px��ʶ��ǰ�ӳ���� ������p �ڷ��ű� �е�λ���±ꣻ-1��ʶmain
	Node* node = new Node("SUBPROG");
	/* code here */
	int tmp_var = 0, tmp_posc = code.size();	// ��¼��һ��jmp�Ĵ����к�
	if (px == -1) {  // main��1��ָ����������ת�����̵ĵ�һ��intλ�ã���ת��ַa �ȴ�����
		emit(6, 0, 0);
	}
	dx = 3;

	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "CONST") {
		node->InsertChild(CONSTANTDECLARE());
	}
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "VAR") {
		int tmp_preVar = symbolTbl.getSize();	// ��¼��ǰ������
		node->InsertChild(VARIABLEDECLARE());
		tmp_var = symbolTbl.getSize() - tmp_preVar;  // ��¼�����ı�����
	}
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
		node->InsertChild(PROCEDUREDECLARE());
	}
	/* code here */
	if (px == -1) {  // ����main��һ��jmp����ת�кţ�Ϊ����emit��int xx
		code[tmp_posc].amt = code.size();
	}
	if (px != -1) {	// ��main���ӳ��򣬷�����ű�para2 ��һ��λ��
		symbolTbl.setPara2(px, code.size());
		SymbolNode tmp_n = symbolTbl.getNode(px);
		for (int i = 0; i < incmpltCode.size(); i++) {
			if (incmpltCode[i].second == tmp_n) {
				code[incmpltCode[i].first].amt = tmp_n.para2;
			}
		}
	}
	emit(5, 0, 3 + tmp_var);  // INT��DX=3������x��VAR��DX+x  Ϊ���򿪱���������ע�⣺����dx+����var���dx��ȫ��������¼������...
	node->InsertChild(SENTENCE());
	/* code here */
	emit(8, 0, 0);   // opr 0 0 ���ظ��ӳ���

	return node;
}
/** 1. CONST
 *========== <����˵������> �� CONST<��������>{ ,<��������>}; ==========
 */
Node* SyntaxAls::CONSTANTDECLARE() {
	Node* node = new Node("CONSTANTDECLARE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "CONST") {
		node->InsertChild(new Node("CONST"));
		++ptr;
		node->InsertChild(CONSTANTDEFINE());
		while (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ",") {
			node->InsertChild(new Node("COMMA"));
			++ptr;
			node->InsertChild(CONSTANTDEFINE());
		}
		if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") { 
			node->InsertChild(new Node(";"));
			++ptr;
			return node; 
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <��������> �� <��ʶ��>=<�޷�������>
 */
Node* SyntaxAls::CONSTANTDEFINE() {
	SymbolNode snode;													/* snode: record symbol */
	snode.kind = "CONSTANT";											// kind
	if (_wordseq[ptr].type == ID) {
		Node* node = new Node("CONSTANTDEFINE");
		node->InsertChild(new Node(_wordseq[ptr].val));
		snode.name = _wordseq[ptr].val;									// name
		++ptr;
		if (_wordseq[ptr].type == OPT && _wordseq[ptr].val == "=") {
			node->InsertChild(new Node("="));
			++ptr;
			if (_wordseq[ptr].type == NUM) {
				node->InsertChild(new Node(_wordseq[ptr].val));
				snode.para1 = stoi(_wordseq[ptr].val);						// para1: VAL
				snode.para2 = cntPro;
				++ptr;
				bool res = symbolTbl.insert(snode, cntPro);					// insert to the table
				if (res == false) {
					smnErr = true;
					return NULL;
				}
				return node;
			}
		}
	}
	synErr = true;
	return NULL;
}
/** 2. VARIABLE
 *========== <����˵������> �� VAR<��ʶ��>{ ,<��ʶ��>}; ==========
 */
Node* SyntaxAls::VARIABLEDECLARE() {
	SymbolNode snode;													/* snode: record symbol */
	snode.kind = "VARIABLE";											// kind
	snode.para1 = cntPro;												// para1: LEVEL==cntPro
	Node* node = new Node("VARIABLEDECLARE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "VAR") {
		node->InsertChild(new Node("VAR"));
		++ptr;
		if (_wordseq[ptr].type == ID) {
			node->InsertChild(new Node(_wordseq[ptr].val));
			snode.name = _wordseq[ptr].val;								// name
			snode.para2 = dx++;												// para2: adr
			bool res = symbolTbl.insert(snode, cntPro);						// insert to the table
			if (res == false) {
				smnErr = true;
				return NULL;
			}
			++ptr;
			while (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ",") {
				node->InsertChild(new Node("COMMA"));
				++ptr; 
				if (_wordseq[ptr].type = ID) {
					node->InsertChild(new Node(_wordseq[ptr].val));
					snode.name = _wordseq[ptr].val;						// name
					snode.para2 = dx++;										// para2: adr
					res = symbolTbl.insert(snode, cntPro);					// insert to the table
					if (res == false) {
						smnErr = true;
						return NULL;
					}
					++ptr;
				}
				else {
					synErr = true;
					return NULL;
				}
			}
			if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") {
				node->InsertChild(new Node(";"));
				++ptr;
				return node;
			}
		}
	}
	synErr = true;
	return NULL;
}
/** 3. PROCEDURE
 *========== <����˵������> �� <�����ײ�><�ֳ���>;{<����˵������>} ==========
 */
Node* SyntaxAls::PROCEDUREDECLARE() {
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
		cntPro++;  // ����һ��
		if (cntPro > 3) {  // ������3��procedure
			synErr = true;
			return NULL;
		}
		Node* node = new Node("PROCEDUREDECLARE");
		node->InsertChild(PROCEDUREHEAD());
		int pos = symbolTbl.getSize() - 1;	// ��ǰpro��ʶ���ڷ��ű��е�λ��
		node->InsertChild(SUBPROG(pos));
		if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") {
			node->InsertChild(new Node(";"));
			cntPro--;  // �ֺ�;���һ��
			++ptr;
			while (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
				node->InsertChild(PROCEDUREDECLARE());
			}
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <�����ײ�> �� PROCEDURE <��ʶ��>;
 */
Node* SyntaxAls::PROCEDUREHEAD() {
	Node* node = new Node("PROCEDUREHEAD");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
		node->InsertChild(new Node("PROCEDURE"));
		++ptr;
		if (_wordseq[ptr].type == ID) {
			node->InsertChild(new Node(_wordseq[ptr].val));
			SymbolNode snode;													/* snode: record symbol */
			snode.kind = "PROCEDURE";											// kind
			snode.name = _wordseq[ptr].val;										// name
			snode.para1 = cntPro - 1;											// para1: LEVEL=cntPro-1 ��Ϊ��һ���ж��Ѿ���ǰ����+1���������������������һ��
			snode.para2 = 0;													// para2: 0 indicates it doesn't have addr yet.
			bool res =symbolTbl.insert(snode, cntPro);							// insert to the table
			if (res == false) {
				smnErr = true;
				return NULL;
			}
			++ptr;
			if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") {
				node->InsertChild(new Node(";"));
				++ptr;
				return node;
			}
		}
	}
	synErr = true;
	return NULL;
}


/**��Ҫ��������������Ŀ�����
 * <���> �� <��ֵ���>|<�������>|<����ѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�����>
 */
Node* SyntaxAls::SENTENCE() {
	Node* node = new Node("SENTENCE");
	if (_wordseq[ptr].type == ID) {
		node->InsertChild(ASSIGNMENT());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "IF") {
		node->InsertChild(IFSENTENCE());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "WHILE") {
		node->InsertChild(WHILESENTENCE());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "CALL") {
		node->InsertChild(CALLSENTENCE());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "READ") {
		node->InsertChild(READSENTENCE());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "WRITE") {
		node->InsertChild(WRITESENTENCE());
	}
	else if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "BEGIN") {
		node->InsertChild(COMBINED());
	}
	else {
		node->InsertChild(EMPTY());
	}
	return node;
}
/**
 * <��ֵ���> �� <��ʶ��>:=<���ʽ>
 */
Node* SyntaxAls::ASSIGNMENT() {
	Node* node = new Node("ASSIGNMENT");
	if (_wordseq[ptr].type == ID) {
		node->InsertChild(new Node(_wordseq[ptr].val));
		string tmp_id = _wordseq[ptr].val;
		++ptr;
		if (_wordseq[ptr].type == OPT && _wordseq[ptr].val == ":=") {
			node->InsertChild(new Node(_wordseq[ptr].val));
			++ptr;
			node->InsertChild(EXPRESSION());
			/* code here */
			int pos = symbolTbl.checkPos(tmp_id, { 2 }, cntPro);
			if (pos < 0) {
				smnErr = true;
				return NULL;
			}
			SymbolNode sym = symbolTbl.getNode(pos);
			emit(3, cntPro - sym.para1, sym.para2);  // STO lev - para1 para2
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <�������> �� IF<����>THEN<���>
 */
Node* SyntaxAls::IFSENTENCE() {
	Node* node = new Node("IFSENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "IF") {
		node->InsertChild(new Node("IF"));
		++ptr;
		node->InsertChild(CONDITION());
		/* code here */
		int tmp_ac = code.size();  // ����a����һ�䣬����-1
		emit(7, 0, 0); // JPC 0 a  ���·�sentence֮����a
		if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "THEN") {
			node->InsertChild(new Node("THEN"));
			++ptr;
			node->InsertChild(SENTENCE());
			code[tmp_ac].amt = code.size();  // ���� ��ת�����֮��Ĵ���λ�ã����Բ���-1
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <����ѭ�����> �� WHILE<����>DO<���>
 */
Node* SyntaxAls::WHILESENTENCE() {
	Node* node = new Node("WHILESENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "WHILE") {
		node->InsertChild(new Node("WHILE"));
		++ptr;
		int tmp_con = code.size();  // ��������һ�䣬����-1
		node->InsertChild(CONDITION());
		/* code here */
		int tmp_ac = code.size();	// ��Ҫ�����a����һ�䣬����-1
		emit(7, 0, 0);  // `JPC 0 a` ����������ת���a��while֮���λ�ã���a��Ҫ��sentence֮����
		if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "DO") {
			node->InsertChild(new Node("DO"));
			++ptr;
			node->InsertChild(SENTENCE());
			/* code here */
			emit(6, 0, tmp_con);  // `JMP 0 a` ��������ת ��<����>����λ��a(tmp_con)
			code[tmp_ac].amt = code.size();  // ����sentence֮���size����-1���Ѿ�����һ��
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <���̵������> �� CALL<��ʶ��>
 */
Node* SyntaxAls::CALLSENTENCE() {
	Node* node = new Node("CALLSENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "CALL") {
		node->InsertChild(new Node("CALL"));
		++ptr;
		if (_wordseq[ptr].type == ID) {
			node->InsertChild(new Node(_wordseq[ptr].val));
			/* code here */
			int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 3 }, cntPro);
			if (pos < 0) {
				smnErr = true;
				return NULL;
			}
			SymbolNode tmp_n = symbolTbl.getNode(pos);
			//cout << "testttt: " << tmp_n.name <<" "<< cntPro - tmp_n.para1<<" "<<tmp_n.para2<< endl << endl;
			if (tmp_n.para2 == 0) {  // ��ǰcall��procedure��û�з����һ��λ�ã�����code��a �ȴ�tmp_n��Ϣ����
				//code.size();
				incmpltCode.push_back(make_pair(code.size(), tmp_n));
			}
			emit(4, cntPro - tmp_n.para1, tmp_n.para2);  // `CAL lev-para1 para2`
			++ptr;
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <�����> �� READ(<��ʶ��>{ ,<��ʶ��>})
 */
Node* SyntaxAls::READSENTENCE() {
	Node* node = new Node("READSENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "READ") {
		node->InsertChild(new Node("READ"));
		++ptr;
		if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == "(") {
			node->InsertChild(new Node("LP"));
			++ptr;
			if (_wordseq[ptr].type == ID) {
				node->InsertChild(new Node(_wordseq[ptr].val));
				/* code here */
				int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 2 }, cntPro);
				if (pos < 0) {
					smnErr = true;
					return NULL;
				}
				SymbolNode tmp_n = symbolTbl.getNode(pos);
				emit(8, 0, 13);  // OPR for READ
				emit(3, cntPro - tmp_n.para1, tmp_n.para2);  // STO
				++ptr;
				while (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ",") {
					node->InsertChild(new Node("COMMA"));
					++ptr;
					if (_wordseq[ptr].type == ID) {
						node->InsertChild(new Node(_wordseq[ptr].val));
						/* code here */
						int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 2 }, cntPro);
						if (pos < 0) {
							smnErr = true;
							return NULL;
						}
						SymbolNode tmp_n = symbolTbl.getNode(pos);
						emit(8, 0, 13);  // OPR for READ
						emit(3, cntPro - tmp_n.para1, tmp_n.para2);  // STO
						++ptr;
					}
					else {
						synErr = true;
						return NULL;
					}
				}
				if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ")") {
					node->InsertChild(new Node("RP"));
					++ptr;
					return node;
				}
			}
		}
		
	}
	synErr = true;
	return NULL;
}
/**
 * <д���> �� WRITE(<��ʶ��>{,<��ʶ��>})
 */
Node* SyntaxAls::WRITESENTENCE() {
	Node* node = new Node("WRITESENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "WRITE") {
		node->InsertChild(new Node("WRITE"));
		++ptr;
		if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == "(") {
			node->InsertChild(new Node("LP"));
			++ptr;
			if (_wordseq[ptr].type == ID) {
				node->InsertChild(new Node(_wordseq[ptr].val));
				/* code here */
				int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 1, 2 }, cntPro);
				if (pos < 0) {
					smnErr = true;
					return NULL;
				}
				SymbolNode tmp_n = symbolTbl.getNode(pos);
				if (tmp_n.kind == "CONSTANT") {
					emit(1, 0, tmp_n.para1);  // LIT
				}
				else if (tmp_n.kind == "VARIABLE") {
					emit(2, cntPro - tmp_n.para1, tmp_n.para2);  // LOD
				}
				emit(8, 0, 14);  // opr 0 14(д���)
				++ptr;
				while (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ",") {
					node->InsertChild(new Node("COMMA"));
					++ptr;
					if (_wordseq[ptr].type == ID) {
						node->InsertChild(new Node(_wordseq[ptr].val));
						/* code here */
						int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 1, 2 }, cntPro);
						if (pos < 0) {
							smnErr = true;
							return NULL;
						}
						SymbolNode tmp_n = symbolTbl.getNode(pos);
						if (tmp_n.kind == "CONSTANT") {
							emit(1, 0, tmp_n.para1);  // LIT
						}
						else if (tmp_n.kind == "VARIABLE") {
							emit(2, cntPro - tmp_n.para1, tmp_n.para2);  // LOD
						}
						emit(8, 0, 14);  // opr 0 14(д���)
						++ptr;
					}
					else {
						synErr = true;
						return NULL;
					}
				}
				if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ")") {
					node->InsertChild(new Node("RP"));
					++ptr;
					return node;
				}
			}
		}

	}
	synErr = true;
	return NULL;
}
/**
 * <�������> �� BEGIN<���>{ ;<���>} END
 */
Node* SyntaxAls::COMBINED() {
	Node* node = new Node("COMBINED");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "BEGIN") {
		node->InsertChild(new Node("BEGIN"));
		++ptr;
		node->InsertChild(SENTENCE());
		while (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") {
			node->InsertChild(new Node(";"));
			++ptr;
			node->InsertChild(SENTENCE());
		}
		if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "END") {
			node->InsertChild(new Node("END"));
			++ptr;
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <�����> �� epsilon
 */
Node* SyntaxAls::EMPTY() {
	Node* node = new Node("EMPTY");
	return node;
}


/**
 * <����> �� <���ʽ><��ϵ�����><���ʽ>|ODD<���ʽ>
 */
Node* SyntaxAls::CONDITION() {
	Node* node = new Node("CONDITION");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "ODD") {
		node->InsertChild(new Node(_wordseq[ptr].val));
		++ptr;
		node->InsertChild(EXPRESSION());
		/* code here `OPR 0 odd(12) */
		emit(8, 0, 12);  // odd
		return node;
	}
	else {
		node->InsertChild(EXPRESSION());
		if (_wordseq[ptr].type == OPT && (_wordseq[ptr].val == "=" || _wordseq[ptr].val == "#" || _wordseq[ptr].val == "<" || _wordseq[ptr].val == ">" || _wordseq[ptr].val == "<=" || _wordseq[ptr].val == ">=")) {
			node->InsertChild(new Node(_wordseq[ptr].val));
			string tmp_opt = _wordseq[ptr].val;
			++ptr;
			node->InsertChild(EXPRESSION());
			/* code here `OPR 0 ��ϵ����� */
			if (tmp_opt == "=") {
				emit(8, 0, 5);
			}
			else if (tmp_opt == "#") {
				emit(8, 0, 6);
			}
			else if (tmp_opt == "<") {
				emit(8, 0, 7);
			}
			else if (tmp_opt == "<=") {
				emit(8, 0, 8);
			}
			else if (tmp_opt == ">") {
				emit(8, 0, 9);
			}
			else if (tmp_opt == ">=") {
				emit(8, 0, 10);
			}
			return node;
		}
		else {
			synErr = true;
			return NULL;
		}
	}
}
/**
 * <���ʽ> �� [+|-]<��>{<�Ӽ������><��>}
 */
Node* SyntaxAls::EXPRESSION() {
	Node* node = new Node("EXPRESSION");
	bool tmp_plus = true;
	if (_wordseq[ptr].type == OPT && (_wordseq[ptr].val == "+" || _wordseq[ptr].val == "-")) {
		if (_wordseq[ptr].val == "-") tmp_plus = false;
		node->InsertChild(new Node(_wordseq[ptr].val));
		++ptr;
	}
	node->InsertChild(ITEM());
	/* code here */
	if (!tmp_plus) {
		emit(8, 0, 11);  // OPR 0 11 (��Ŀ-)
	}
	while (_wordseq[ptr].type == OPT && (_wordseq[ptr].val == "+" || _wordseq[ptr].val == "-")) {
		node->InsertChild(new Node(_wordseq[ptr].val));
		if (_wordseq[ptr].val == "-") tmp_plus = false;
		else tmp_plus = true;
		++ptr;
		node->InsertChild(ITEM());
		/* code here */
		if (!tmp_plus) {
			emit(8, 0, 2);  // OPR 0 2 -
		}
		else {
			emit(8, 0, 1);
		}
	}
	return node;
}
/**
 * <��> �� <����>{<�˳������><����>}
 */
Node* SyntaxAls::ITEM() {
	Node* node = new Node("ITEM");
	node->InsertChild(FACTOR());
	bool tmp_mul;
	while (_wordseq[ptr].type == OPT && (_wordseq[ptr].val == "*" || _wordseq[ptr].val == "/")) {
		if (_wordseq[ptr].val == "*") tmp_mul = true;
		else tmp_mul = false;
		node->InsertChild(new Node(_wordseq[ptr].val));
		++ptr;
		node->InsertChild(FACTOR());
		/* code here */
		if(tmp_mul) emit(8, 0, 3);  // OPR 0 3 / 4[*|/]
		else emit(8, 0, 4);
	}
	return node;
}
/**
 * <����> �� <��ʶ��>|<�޷�������>|(<���ʽ>)
 */
Node* SyntaxAls::FACTOR() {
	Node* node = new Node("FACTOR");
	if (_wordseq[ptr].type == ID || _wordseq[ptr].type == NUM) {
		node->InsertChild(new Node(_wordseq[ptr].val));
		/* code here */
		if(_wordseq[ptr].type == NUM)  // unsigned integer
			emit(1, 0, stoi(_wordseq[ptr].val));  // LIT 0 ����
		else {  // identifier
			//int pos = symbolTbl.checkPos(_wordseq[ptr].val, cntPro);
			int pos = symbolTbl.checkPos(_wordseq[ptr].val, { 1,2 }, cntPro);
			if (pos < 0) {
				smnErr = true;
				return NULL;
			}
			SymbolNode sym = symbolTbl.getNode(pos);
			if (sym.kind == "CONSTANT") {
				emit(1, 0, sym.para1);  // const  `LIT 0 para1`
			}
			else if (sym.kind == "VARIABLE") {
				emit(2, cntPro - sym.para1, sym.para2);  //var  `LOD lev - para1 para2`
			}
			else {
				smnErr = true;
				return NULL;
			}
		}
		++ptr;
		return node;
	}
	else if (_wordseq[ptr].type == SEP || _wordseq[ptr].val == "(") { 
		node->InsertChild(new Node("LP"));
		++ptr;
		node->InsertChild(EXPRESSION());
		if (_wordseq[ptr].type == SEP || _wordseq[ptr].val == ")") {
			node->InsertChild(new Node("RP"));
			++ptr;
			return node;
		}
		synErr = true;
		return NULL;
	}
	else {
		synErr = true;
		return NULL;
	}
}
