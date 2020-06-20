#pragma once
#include <string>
#include "SematicProcesser.h"



namespace hscp {
	

	struct Quad
	{
		std::string op;//operator
		std::string addr1;//opnd1
		std::string addr2;//opnd2
		std::string addr3;//opnd3


		Quad(std::string op, std::string a1 = "_", std::string a2 = "_", std::string a3 = "_") : op(op), addr1(a1), addr2(a2), addr3(a3)
		{
		}
	};
	void PrintQuads(const std::vector<Quad>& qs) {
		for (const auto& q : qs) {
			std::cout << "(" << std::setw(6) << q.op << ", " << std::setw(6) << q.addr1 << ", " << std::setw(6) << q.addr2 << ", " << std::setw(6) << q.addr3 << ")\n";
		}
	}
	

	class IR {
	private:
		//const AST& ast;
		std::vector<Quad> irCode;
		int tempVarCount = 0;
		int labelCount = 0;

		const std::string tempVar = "_t";
		const std::string label = "_lb";

		const std::map < std::string, std::string > operator2name = {
			{"+","ADD"},
			{"-","MINUS"},
			{"*","MUL"},
			{"/","DIV"},
			{">","GT"},
			{"<","LS"},
			{"=","EQ"}
		};

		//IR(const AST& ast) : ast(ast) { }
		//void traverse() {
			//irCode = parseTree(ast.root);
		//}
		std::vector<Quad> parseTree(ASTNode* node) {
			if (node == nullptr) return{};
			std::vector<std::vector<Quad>> ts;
			for (const auto& cn : node->children) {
				auto t = parseTree(cn);
				if (t.size() > 0)
					ts.push_back(t);
			}

			if (node->op == "sequence") {
				std::vector<Quad> qs;
				for (const auto& q : ts) {
					std::copy(q.begin(), q.end(), std::back_inserter(qs));
				}
				return qs;
			}
			else if (node->op == "ID") {
				return { Quad("MOVE", node->val, node->val) };
			}
			else if (node->op == "NUM") {
				return { Quad("MOVE",node->val,tempVar + std::to_string(tempVarCount++)) };

			}
			else if (node->op == "READ" || node->op == "WRITE") {
				std::vector<Quad> qs;
				std::copy(ts[0].begin(), ts[0].end(), std::back_inserter(qs));
				qs.push_back(Quad(node->op, ts[0].back().addr2));
				return qs;
			}
			else if (node->op == "ASSIGN") {
				std::vector<Quad> qs;
				std::copy(ts[0].begin(), ts[0].end(), std::back_inserter(qs));
				std::copy(ts[1].begin(), ts[1].end(), std::back_inserter(qs));
				qs.push_back(Quad("MOVE", ts[1].back().addr2, ts[0].back().addr2));
				return qs;
			}
			else if (node->op == "IF") {
				std::vector<Quad> qs;
				std::copy(ts[0].begin(), ts[0].end(), std::back_inserter(qs));
				auto label_false = labelCount++;
				qs.push_back(Quad("IF_F", ts[0].back().addr2, label + std::to_string(label_false)));
				std::copy(ts[1].begin(), ts[1].end(), std::back_inserter(qs));
				auto label_end = labelCount++;
				qs.push_back(Quad("JMP", label + std::to_string(label_end)));
				qs.push_back(Quad("LABEL", label + std::to_string(label_false)));
				if (ts.size() > 2) {
					std::copy(ts[2].begin(), ts[2].end(), std::back_inserter(qs));
				}
				qs.push_back(Quad("LABEL", label + std::to_string(label_end)));
				return qs;
			}
			else if (node->op == "REPEAT_DO") {
				std::vector<Quad> qs;
				auto label_repeat_begin = labelCount++;
				qs.push_back(Quad("LABEL", label + std::to_string(label_repeat_begin)));
				std::copy(ts[0].begin(), ts[0].end(), std::back_inserter(qs));

				std::copy(ts[1].begin(), ts[1].end(), std::back_inserter(qs));
				qs.push_back(Quad("IF_F", ts[1].back().addr2, label + std::to_string(label_repeat_begin)));

				return qs;
			}
			else if (node->val == "") {
				std::vector<Quad> qs;
				std::copy(ts[0].begin(), ts[0].end(), std::back_inserter(qs));
				std::copy(ts[1].begin(), ts[1].end(), std::back_inserter(qs));

				qs.push_back(Quad(operator2name.at(node->op), ts[0].back().addr2, ts[1].back().addr2, tempVar + std::to_string(tempVarCount++)));
				qs.push_back(Quad("MOVE", qs.back().addr3, tempVar + std::to_string(tempVarCount++)));

				return qs;
			}
		}
		/*std::vector<Quad> parseExp(ASTNode* node) {
			std::set<std::vector<Quad>> ts;
			for (const auto& cn : node->children) {
				ts.insert(parseTree(cn));
			}

		}*/

	public:
		static std::vector<Quad> GenIR(const AST& ast) {
			IR irc;
			//irc.traverse();
			return irc.parseTree(ast.root);
		}
	};
}