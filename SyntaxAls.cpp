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
 * <程序>→<分程序>.
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
/** 语义分析：3个说明部分建立名字表；语句部分逐语句分析；
 * <分程序>→ [<常量说明部分>][<变量说明部分>][<过程说明部分>]<语句>
 */
Node* SyntaxAls::SUBPROG(int px) {	// px标识当前子程序的 过程名p 在符号表 中的位置下标；-1标识main
	Node* node = new Node("SUBPROG");
	/* code here */
	int tmp_var = 0, tmp_posc = code.size();	// 记录第一条jmp的代码行号
	if (px == -1) {  // main第1条指令无条件跳转到过程的第一条int位置，跳转地址a 等待反填
		emit(6, 0, 0);
	}
	dx = 3;

	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "CONST") {
		node->InsertChild(CONSTANTDECLARE());
	}
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "VAR") {
		int tmp_preVar = symbolTbl.getSize();	// 记录当前符号数
		node->InsertChild(VARIABLEDECLARE());
		tmp_var = symbolTbl.getSize() - tmp_preVar;  // 记录声明的变量数
	}
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
		node->InsertChild(PROCEDUREDECLARE());
	}
	/* code here */
	if (px == -1) {  // 反填main第一条jmp的跳转行号，为如下emit的int xx
		code[tmp_posc].amt = code.size();
	}
	if (px != -1) {	// 非main的子程序，反填符号表para2 第一句位置
		symbolTbl.setPara2(px, code.size());
		SymbolNode tmp_n = symbolTbl.getNode(px);
		for (int i = 0; i < incmpltCode.size(); i++) {
			if (incmpltCode[i].second == tmp_n) {
				code[incmpltCode[i].first].amt = tmp_n.para2;
			}
		}
	}
	emit(5, 0, 3 + tmp_var);  // INT（DX=3），有x个VAR，DX+x  为程序开辟数据区；注意：别用dx+，在var里把dx当全局用来记录数量了...
	node->InsertChild(SENTENCE());
	/* code here */
	emit(8, 0, 0);   // opr 0 0 返回该子程序

	return node;
}
/** 1. CONST
 *========== <常量说明部分> → CONST<常量定义>{ ,<常量定义>}; ==========
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
 * <常量定义> → <标识符>=<无符号整数>
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
 *========== <变量说明部分> → VAR<标识符>{ ,<标识符>}; ==========
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
 *========== <过程说明部分> → <过程首部><分程序>;{<过程说明部分>} ==========
 */
Node* SyntaxAls::PROCEDUREDECLARE() {
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "PROCEDURE") {
		cntPro++;  // 创建一层
		if (cntPro > 3) {  // 不大于3层procedure
			synErr = true;
			return NULL;
		}
		Node* node = new Node("PROCEDUREDECLARE");
		node->InsertChild(PROCEDUREHEAD());
		int pos = symbolTbl.getSize() - 1;	// 当前pro标识符在符号表中的位置
		node->InsertChild(SUBPROG(pos));
		if (_wordseq[ptr].type == SEP && _wordseq[ptr].val == ";") {
			node->InsertChild(new Node(";"));
			cntPro--;  // 分号;完成一层
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
 * <过程首部> → PROCEDURE <标识符>;
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
			snode.para1 = cntPro - 1;											// para1: LEVEL=cntPro-1 因为上一个判断已经提前层数+1，但是这个过程名属于上一层
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


/**需要逐语句分析，生成目标代码
 * <语句> → <赋值语句>|<条件语句>|<当型循环语句>|<过程调用语句>|<读语句>|<写语句>|<复合语句>|<空语句>
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
 * <赋值语句> → <标识符>:=<表达式>
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
 * <条件语句> → IF<条件>THEN<语句>
 */
Node* SyntaxAls::IFSENTENCE() {
	Node* node = new Node("IFSENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "IF") {
		node->InsertChild(new Node("IF"));
		++ptr;
		node->InsertChild(CONDITION());
		/* code here */
		int tmp_ac = code.size();  // 反填a是下一句，不用-1
		emit(7, 0, 0); // JPC 0 a  在下方sentence之后反填a
		if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "THEN") {
			node->InsertChild(new Node("THEN"));
			++ptr;
			node->InsertChild(SENTENCE());
			code[tmp_ac].amt = code.size();  // 反填 跳转至语句之后的代码位置，所以不用-1
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <当型循环语句> → WHILE<条件>DO<语句>
 */
Node* SyntaxAls::WHILESENTENCE() {
	Node* node = new Node("WHILESENTENCE");
	if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "WHILE") {
		node->InsertChild(new Node("WHILE"));
		++ptr;
		int tmp_con = code.size();  // 条件在下一句，不用-1
		node->InsertChild(CONDITION());
		/* code here */
		int tmp_ac = code.size();	// 需要反填的a在下一句，不用-1
		emit(7, 0, 0);  // `JPC 0 a` 条件非真跳转语句a（while之外的位置），a需要在sentence之后反填
		if (_wordseq[ptr].type == SYM && _wordseq[ptr].val == "DO") {
			node->InsertChild(new Node("DO"));
			++ptr;
			node->InsertChild(SENTENCE());
			/* code here */
			emit(6, 0, tmp_con);  // `JMP 0 a` 无条件跳转 到<条件>代码位置a(tmp_con)
			code[tmp_ac].amt = code.size();  // 总在sentence之后反填，size不用-1，已经是下一句
			return node;
		}
	}
	synErr = true;
	return NULL;
}
/**
 * <过程调用语句> → CALL<标识符>
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
			if (tmp_n.para2 == 0) {  // 当前call的procedure还没有反填第一句位置，此条code的a 等待tmp_n信息反填
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
 * <读语句> → READ(<标识符>{ ,<标识符>})
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
 * <写语句> → WRITE(<标识符>{,<标识符>})
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
				emit(8, 0, 14);  // opr 0 14(写语句)
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
						emit(8, 0, 14);  // opr 0 14(写语句)
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
 * <复合语句> → BEGIN<语句>{ ;<语句>} END
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
 * <空语句> → epsilon
 */
Node* SyntaxAls::EMPTY() {
	Node* node = new Node("EMPTY");
	return node;
}


/**
 * <条件> → <表达式><关系运算符><表达式>|ODD<表达式>
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
			/* code here `OPR 0 关系运算符 */
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
 * <表达式> → [+|-]<项>{<加减运算符><项>}
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
		emit(8, 0, 11);  // OPR 0 11 (单目-)
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
 * <项> → <因子>{<乘除运算符><因子>}
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
 * <因子> → <标识符>|<无符号整数>|(<表达式>)
 */
Node* SyntaxAls::FACTOR() {
	Node* node = new Node("FACTOR");
	if (_wordseq[ptr].type == ID || _wordseq[ptr].type == NUM) {
		node->InsertChild(new Node(_wordseq[ptr].val));
		/* code here */
		if(_wordseq[ptr].type == NUM)  // unsigned integer
			emit(1, 0, stoi(_wordseq[ptr].val));  // LIT 0 整数
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
