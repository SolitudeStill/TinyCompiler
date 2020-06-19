#include"LexFileLoader.h"
#include"Automaton.h"
#include"GrammarFileReader.h"
#include"LRAutos.h"
#include"LRAnalyzer.h"
#include "DFA.h"
#include "LexMatcher.h"
#include "SematicLoader.h"
#include "SematicProcesser.h"
#include "intermediate.h"
using namespace std;
// get an regex automaton
hscp::Automaton getAutos() {
#ifdef _DEBUG
	constexpr auto route = "Data\\lex-define.txt";
#else
	constexpr auto route = "lex-define.txt";
#endif

	hscp::Automaton at;
	// load expressions
	hscp::FileLoader(route, [](const auto& err) {}, [&at](const vector<hscp::token_define>& defs) {
		for (const auto& d : defs) { // for each expression
			// convert to postfix expression then to NFA
			auto nfa = hscp::Automaton::RegexPost2NFA(hscp::RegexProcesser::ProcessRegex(d.expr), d.id);
			// to DFA
			auto dfa = hscp::DFAConverter::Nfa2Dfa(nfa);
			// minimize
			auto mindfa = hscp::DFAminimizer(dfa);
			// merge automatons to one big automaton
			at = hscp::Automaton::Merge(at, mindfa);
		}
		});

	at = hscp::DFAConverter::Nfa2Dfa(at);  // there're epsilons and transitions accept same inputs after merge

	return std::move(at);
}
int main(int argc, char** argv) {
	string file = "Data\\source.txt";
	//if (argc == 2)
	//	file = argv[1]; // source file from parameter
	//else return 0;

	auto at = getAutos();
	// init matcher
	hscp::Matcher mc(at);
	// begin match
	auto tokens = mc.ReadFile(file);

	// load grammar
	hscp::GrammarLoader ld;
	//ld.Print();
	ld.EnableLR(); // in GrammarFileReader.h , there's a constant identifies the start symbol for grammar
	//ld.Print();
	auto lrat = hscp::LR1Automaton::Build(ld);
	
	auto t = lrat.LR1Table();
	
	hscp::Analyzer ana(lrat, t, tokens);
	ana.PrintErrors();

	auto& atree = ana.GetAnalyzeTree();
	std::set<std::string> symbol_table;
	hscp::SematicLoader sematic;
	auto ast = hscp::SematicProcesser::AnalyzeToAST(sematic, atree, symbol_table);
	atree.Destroy(); // dertroy analyze tree
	hscp::PrintAST(ast);
	genIR medCode (ast);//生成中间代码
	return 0;
}
