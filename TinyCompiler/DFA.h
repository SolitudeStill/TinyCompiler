#pragma once
#include<map>
#include<set>
#include<functional>

#include"Automaton.h"


namespace hscp {
	class DFAConverter {
	private:
		const Automaton& nfa;
		Automaton dfa;
		std::map<std::set<state*>, state*> states_to_dfa_s; // map nfa states to dfa state
		std::set<std::set<state*>> visited; // store visited [states] in state construction
		std::set<CharRange> inputs; // all valid input

		// get input from nfa transitions
		void getInputs() {
			// traverse trasition to get inputs
			for (const auto& t : nfa.transitions) {
				if (t->input != 0) {
					if (t->input.isSingle()) // not a range
						inputs.emplace(t->input);
					else // is range
						for (int i = t->input.from; i <= t->input.to; i++)
							inputs.emplace(i);
				}
			}
		}
		// tell a set of states is visited
		bool isVisited(const std::set<state*>& ss) {
			return visited.find(ss) != visited.end();
		}
		// for each state's transition do an action
		void iterateTransitions(std::set<state*> states, std::function<void(state*, transition*)> ac) {
			for (const auto& s : states) {
				if (s->trans.size() == 0)
					ac(s, nullptr); // for states have no transitions out
				else
					for (const auto& t : s->trans) {
						ac(s, t);
					}
			}
		}
		// generate e-closure set of states
		void epsilonClosure(const std::set<state*>& states, std::set<state*>& closed) {
			iterateTransitions(states, [this, &closed](state* s, transition* t) { // for each transition do
				closed.insert(s); // add the state itself

				if (t != nullptr && t->input == 0 && closed.find(t->to) == closed.end()) { // has epsilon transition to state not included
					closed.insert(t->to); // add state

					epsilonClosure({ t->to }, closed); // continue closure
				}
				});
		}
		// from current state read a input and go to next
		void toAdvance(const std::set<state*>& states, std::set<state*>& dst, unsigned char input) {
			iterateTransitions(states, [&dst, input](state* s, transition* t) { // for each transition do
				if (t != nullptr && t->input.from <= input && t->input.to >= input) { // contain valid trasition
					dst.insert(t->to); // one of the next state, add to destination set
				}
				});
		}
		// get a not visited set of states from all
		std::set<state*> getNonVisited() {
			for (const auto& ss : states_to_dfa_s) {
				if (!isVisited(ss.first)) {
					return ss.first;
				}
			}
			return {}; // no not visited
		}
		// set states in dfa final states, which contains final state in nfa
		void setEndStates() {
			for (const auto& ss_s : states_to_dfa_s) {
				std::string is;
				ss_s.second->finalState = ss_s.first.end() != std::find_if(ss_s.first.begin(), ss_s.first.end(), [&ss_s](state* s) {
					if (s->finalState) {
						ss_s.second->is = s->is;
						return true;
					}
					return false;
				}); // find a final state and get its lexical meaning

			}
		}
		// converter
		Automaton dfaConverter() {
			dfa.startState = dfa.NewState();
			std::set<state*> t;

			epsilonClosure({ nfa.startState }, t);
			states_to_dfa_s.emplace(t, dfa.startState); // getting initial state

			std::set<state*> ss;  // nonvisited states set
			std::set<state*> empty = {}; // to compare

			while ((ss = getNonVisited()) != empty) // get state left
			{
				visited.insert(ss); // now this set is visited
				for (const auto& i : inputs) { // for each input get a new transition/state(set)
					std::set<state*> t1, t2;
					// move to next
					toAdvance(ss, t1, i);

					epsilonClosure(t1, t2);

					if (t2 == empty) continue;

					if (!isVisited(t2)) { // it's a new state
						if (states_to_dfa_s.find(t2) == states_to_dfa_s.end()) {
							state* n = dfa.NewState();
							states_to_dfa_s.emplace(t2, n);
						}
					}
					// whatever, there's new transition
					dfa.NewTransition(states_to_dfa_s[ss], states_to_dfa_s[t2], i, "");
				}
			}
			setEndStates();
			return std::move(dfa);
		}
		// convert to dfa
		DFAConverter(const Automaton& nfa) :nfa(nfa) {
			// get all possible input
			getInputs();
			// inner converter
			dfa = dfaConverter();
			// set inputs
			dfa.inputs = this->inputs;
		}
		// export dfa
		Automaton Export() {
			return this->dfa;
		}
	public:
		// convert nfa to dfa
		static Automaton Nfa2Dfa(const Automaton& nfa) {
			return DFAConverter(nfa).Export();
		}
	};

	// minimizer
	Automaton DFAminimizer(const Automaton& dfa) {
		Automaton mindfa;
		std::vector<std::set<state*>> classified; // sets of equivalent states
		// get set index in classified group local function
		auto getSetno = [&classified](state* s) {
			for (size_t i = 0; i < classified.size(); i++)
			{
				if (classified[i].find(s) != classified[i].end()) {
					return i;
				}
			}
		};
		// tell if two state is equal
		auto isEqual = [&dfa, &classified, getSetno](state* s1, state* s2) {
			std::map<unsigned char, int> setno1, setno2;
			for (const auto& i : dfa.inputs) { // init destination map
				setno1[i] = -1;
				setno2[i] = -1;
			}
			for (const auto& t : s1->trans) {
				setno1[t->input] = getSetno(t->to);
			}
			for (const auto& t : s2->trans) {
				setno2[t->input] = getSetno(t->to);
			}
			return setno1 == setno2;
		};
		// divide a group of states not equal to the first state in current set
		auto divide = [&classified, isEqual](std::set<state*>& s) {
			if (s.size() < 2) return false; // won't divide this turn
			auto f = *(s.begin()); // first state in set
			std::set<state*> nef; // not equal to the first state in set
			for (auto i = s.begin(); i != s.end(); ++i) {
				if (i == s.begin()) continue;
				else {
					if (!isEqual(f, *i)) { // not equal to first state
						nef.insert(*i);
					}
				}
			}
			for (const auto& nes : nef) {
				s.erase(nes); // remove divided state
			}
			if (nef.size() > 0)
			{
				classified.emplace_back(nef); // push new set
				return true;
			}
			else return false;
		};
		// first division
		std::set<state*> t1, t2;
		for (const auto& s : dfa.states) { // divide final state with nonfinal
			if (s->finalState)
				t2.insert(s.Obj());
			else
				t1.insert(s.Obj());
		}
		classified.emplace_back(t1); t1.clear();
		classified.emplace_back(t2); t2.clear();

		bool divided = true;
		while (divided) // if last turn divided new set, do another turn
		{
			divided = false;
			for (size_t i = 0; i < classified.size(); i++) // for each group
			{
				divided |= divide(classified[i]); // if divided
			}
		}

		std::map<std::set<state*>, state*> dfa_ss_to_min_s;
		for (const auto& s : classified) { // generate a map from states to minimized state
			dfa_ss_to_min_s.emplace(s, mindfa.NewState()); 
			if ((*s.begin())->finalState) { // update final state record
				dfa_ss_to_min_s[s]->finalState = true;
				dfa_ss_to_min_s[s]->is = (*s.begin())->is;
			}
			if (s.find(dfa.startState) != s.end()) { // set start state
				mindfa.startState = dfa_ss_to_min_s[s];
			}
		}
		for (const auto& s : dfa_ss_to_min_s) { // generate new transitions
			for (const auto& t : (*s.first.begin())->trans) { // transitions of a group of state
				std::string tis = "";
				if (t->to->finalState)
					tis = t->to->is;
				mindfa.NewTransition(s.second, dfa_ss_to_min_s[classified[getSetno(t->to)]], t->input, tis);
			}
		}
		return std::move(mindfa);
	}
}