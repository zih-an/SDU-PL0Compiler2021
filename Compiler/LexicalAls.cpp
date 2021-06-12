#include "LexicalAls.h"

LexicalAls::LexicalAls() {
	id_state = 0, dgt_state = 0, opt_state = 0;
	crct_id = true, crct_dgt = true, crct_opt = true;
	lxcError = false;
}

void LexicalAls::restart() {
	idntf = ""; dgt = ""; opt = "";
	id_state = 0, dgt_state = 0, opt_state = 0;
	crct_id = true, crct_dgt = true, crct_opt = true;
	lxcError = false;
	outputs.clear();
}

bool LexicalAls::Identifier(char& c) {
	bool crct = true;
	if (!isalnum(c)) { crct = false; }
	else {
		switch (id_state) {
		case 0:
			if (isalpha(c)) {
				idntf += c;
				id_state += 1;
			}
			else {
				crct = false;
			}
			break;
		case 1:
			idntf += c;
			id_state += 1;
			break;
		case 2:
			idntf += c;
			break;
		}
	}

	return crct;
}
bool LexicalAls::Digit(char& c) {
	bool crct = true;
	if (!isdigit(c)) { crct = false; }
	else {
		switch (dgt_state) {
		case 0:
			dgt += c;
			dgt_state += 1;
			break;
		case 1:
			dgt += c;
			break;
		}
	}

	//if(state != 1) { crct = false; }
	return crct;
}
bool LexicalAls::checkKeys() {
	for (auto word : keywords) {
		if (word == idntf) {
			return true;
		}
	}
	return false;
}
bool LexicalAls::checkSeps(char& c) {
	for (auto sep : separators) {
		if (sep == c) {
			return true;
		}
	}
	return false;
}
bool LexicalAls::checkOpts(char& c) {
	bool crct = false;
	switch (opt_state) {
	case 0:
		for (auto opt1 : opts_st1) {
			if (opt1 == c) {
				crct = true;
				opt += c;
				opt_state += 1;
				if (c == '<' || c == '>' || c == ':') {
					opt_state += 1;
				}
				break;
			}
		}
		break;
	case 1:
		crct = false; // wrong
		break;
	case 2:
		if (c == '=') {
			opt += c;
			crct = true;
		}
		break;
	}
	return crct;
}

void LexicalAls::checkState() {
	if (id_state != 1 && id_state != 2) {
		crct_id = false;
	}
	if (dgt_state != 1) {
		crct_dgt = false;
	}
	if (opt_state != 1 && opt_state != 2) {
		crct_opt = false;
	}
}
void LexicalAls::output() {
	if (idntf.size() > 10 || dgt.size() > 10 || opt.size() > 10) {
		lxcError = true;
		return;
	}

	if (crct_id) {
		if (checkKeys()) {
			outputs.emplace_back(SYM, idntf);
		}
		else {
			outputs.emplace_back(ID, idntf);
		}
	}
	else if (crct_dgt) {
		int tmp = stoi(dgt);
		outputs.emplace_back(NUM, to_string(tmp));
	}
	else if (crct_opt) {
		if (find(operators.begin(), operators.end(), opt) != operators.end()) {
			outputs.emplace_back(OPT, opt);
		}
		else lxcError = true;
	}
	else {
		if (idntf.empty() && dgt.empty() && opt.empty());
		else lxcError = true;
	}
}
void LexicalAls::clear() {
	idntf = ""; dgt = ""; opt = "";
	id_state = 0, dgt_state = 0, opt_state = 0;
	crct_id = true, crct_dgt = true, crct_opt = true;
}

void LexicalAls::run() {
	char c;
	c = getchar();
	while (c != EOF) {
		c = toupper(c);
		if (c == ' ' || c == '\n' || c == '\t' || checkSeps(c)) {  // refresh: output&clear or nothing
			checkState();
			output();
			clear();
			if (checkSeps(c)) outputs.emplace_back(SEP, string(1, c));
			c = getchar();
			continue;
		}

		if (crct_opt) {  //opt=1
			crct_opt = checkOpts(c);
		}
		if (crct_id) { //id=2
			crct_id = Identifier(c);
		}
		if (crct_dgt) { //dgt=3
			crct_dgt = Digit(c);
		}

		int opt = 0;
		if (crct_opt) opt = 1;
		if (crct_id) opt = 2;
		if (crct_dgt) opt = 3;

		switch (opt) {
		case 0:
			lxcError = true;
			c = getchar();
			break;
		case 1:
			while (crct_opt) {
				c = getchar();
				c = toupper(c);
				crct_opt = checkOpts(c);
			}
			crct_opt = true;
			break;
		case 2:
			while (crct_id) {
				c = getchar();
				c = toupper(c);
				crct_id = Identifier(c);
			}
			crct_id = true;
			break;
		case 3:
			while (crct_dgt) {
				c = getchar();
				c = toupper(c);
				crct_dgt = Digit(c);
			}
			if (!isalpha(c)) { crct_dgt = true; }
			else { c = getchar(); }
			break;
		}

		//output
		checkState();
		output();
		clear();
	}

	//printout();
}