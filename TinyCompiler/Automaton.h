#pragma once
#include<vector>
#include<string>
#include<set>
#include<map>
#include<filesystem>
#include<iostream>
#include<fstream>

#include"Vlpp.h"
#include"RegExpParser.h"

namespace hscp {
	struct state;
	struct transition;
	// a state in automaton
	struct state {
		//std::vector<transition*> ins; // transition comes in
		std::vector<transition*> trans; // transition functions
		bool finalState; // is final
		std::string is; // the lexical meaning which this function belongs to
	};
	// a transition in automaton; both used to record a fragment in automaton
	struct transition {
		CharRange input; // function input
		std::string is; // the lexical meaning which this function belongs to
		state* from, * to; // where the function comes and goes
	};
	// a automaton
	class Automaton {
	public:
		std::vector<vl::Ptr<state>> states;
		std::vector<vl::Ptr<transition>> transitions;
		std::set<CharRange> inputs;
		state* startState = nullptr; // the very beginning of this automaton

		// add a state
		state* NewState(const std::string& is = "") {
			state* t = new state{ {},false, is };
			states.push_back(t);
			return t;
		}
		// add a transition
		transition* NewTransition(state* from, state* to, CharRange chr, const std::string& is) {
			transition* t = new transition{ chr,is,from,to };
			from->trans.push_back(t);
			//to->ins.push_back(t);
			transitions.push_back(t);
			return t;
		}
		// add a transition takes epsilon
		transition* NewEpsilon(state* from, state* to) {
			return NewTransition(from, to, 0, "");
		}
		// print all transitions
		void print() {
			for (const auto& t : transitions) {
				std::cout << t->from << "  [" << t->input << "]  " << t->to << "  " << (t->to->finalState) << '\n';
			}
		}
		
		// generate nfa from postfix regular expression
		static Automaton RegexPost2NFA(const std::vector<regextok>& regex, const std::string& is) {
			std::stack<transition*> subAutos; // a stack for fragments
			auto newSubs = [](state* begin, state* end) { // local function to create a fragment
				transition* t = new transition{ 255,"",begin,end }; // record start and end state of the fragment
				return t;
			};
			Automaton nfa;

			transition* sub1, * sub2;
			state* st, * ed;
			for (auto i = regex.begin(); i != regex.end(); ++i) {
				if (*i == '|') { // parallel two fragments
					// get fragments
					sub1 = subAutos.top(); subAutos.pop();
					sub2 = subAutos.top(); subAutos.pop();
					// create start and end state
					st = nfa.NewState();
					ed = nfa.NewState();
					// use epsilon build fragment
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(st, sub2->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub2->to, ed);

					subAutos.push(newSubs(st, ed)); // push fragment
				}
				else if (*i == '&') { // concatenate two fragments
					sub1 = subAutos.top(); subAutos.pop(); // right
					sub2 = subAutos.top(); subAutos.pop(); // left
					// use epsilon connect fragments
					nfa.NewEpsilon(sub2->to, sub1->from);

					subAutos.push(newSubs(sub2->from, sub1->to)); // push fragment
				}
				else if (*i == '*') { // closure
					sub1 = subAutos.top(); subAutos.pop(); // the symbol

					st = nfa.NewState();
					ed = nfa.NewState();
					// use epsilon build fragment
					nfa.NewEpsilon(st, ed);
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub1->to, sub1->from);

					subAutos.push(newSubs(st, ed)); // push fragment
				}
				else if (*i == '+') { // positive closure
					sub1 = subAutos.top(); subAutos.pop(); // the symbol

					st = nfa.NewState();
					ed = nfa.NewState();
					// use epsilon build fragment
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub1->to, sub1->from);

					subAutos.push(newSubs(st, ed)); // push fragment
				}
				else if (*i == '?') { // optional
					sub1 = subAutos.top(); subAutos.pop(); // the symbol

					st = nfa.NewState();
					ed = nfa.NewState();
					// use epsilon build fragment
					nfa.NewEpsilon(st, ed);
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);

					subAutos.push(newSubs(st, ed)); // push fragment
				}
				else // normal character
				{
					st = nfa.NewState();
					ed = nfa.NewState();
					// build char transition
					nfa.NewTransition(st, ed, *i, is);

					subAutos.push(newSubs(st, ed)); // push fragment
				}
			}

			// the fragment left is whole automaton
			nfa.startState = subAutos.top()->from; // set start
			subAutos.top()->to->finalState = true; // set finish
			subAutos.top()->to->is = is;
			return std::move(nfa);
		}
		// merge two parallel automaton
		static Automaton Merge(Automaton& nfa1, Automaton& nfa2) {
			Automaton nfa;
			auto st = nfa.NewState();
			nfa.startState = st;

			if (nfa1.startState != nullptr) { // check empty automaton
				// automaton1 to new automaton
				nfa.NewEpsilon(st, nfa1.startState); 
				std::copy(nfa1.states.begin(), nfa1.states.end(), std::back_inserter(nfa.states));
				std::copy(nfa1.transitions.begin(), nfa1.transitions.end(), std::back_inserter(nfa.transitions));
			}
			if (nfa2.startState != nullptr) { // check empty automaton
				// automaton2 to new automaton
				nfa.NewEpsilon(st, nfa2.startState); 
				std::copy(nfa2.states.begin(), nfa2.states.end(), std::back_inserter(nfa.states));
				std::copy(nfa2.transitions.begin(), nfa2.transitions.end(), std::back_inserter(nfa.transitions));
			}

			return std::move(nfa);
		}
	};
}