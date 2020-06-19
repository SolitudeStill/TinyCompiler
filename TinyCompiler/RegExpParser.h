#pragma once
#include<array>
#include<vector>
#include<bitset>
#include<map>
#include<stack>

#include"Vlpp.h"
namespace hscp {
	// a range for chars
	struct CharRange {
		unsigned char from, to;
		CharRange(unsigned char ch) :from(ch), to(ch) {}
		CharRange(unsigned char from, unsigned char to) :from(from), to(to) {}
		operator unsigned char() const {
			return isSingle() ? from : -1;
		}
		// tell if this contains single character
		bool isSingle() const { return from == to; }
		bool Match(char ch) {
			bool inrange = ch >= from && ch <= to;
			return inrange;
		}
	};

	// unit of regular expression
	class regextok {
	public:
		CharRange chr; // base char
		bool escaped; // if it's a escaped character

		regextok(unsigned char c, bool esc = false) :chr(c), escaped(esc) {}
		regextok(unsigned char from, unsigned char to) :chr(from, to), escaped(false) {}
		regextok(const CharRange& chr) :chr(chr), escaped(false) {}
		bool operator==(char c) const { // compare with reserve character to know if the char is a operator
			return !escaped && chr == c;
		}

		operator unsigned char() const { // convert to a char
			return chr;
		}
		operator CharRange() const { // convert to a char range
			return chr;
		}
	};

	class RegexProcesser {
	private:
		
		// process escaped chars
		static std::vector<regextok> escapeChar(unsigned char ch) {
			std::vector<regextok> toks;
			switch (ch)
			{
			case 'd': // \d in common regex
				toks = { '[','0','-','9',']' };
				break;
			case 'D': // \D in common regex
				toks = { '[','^','0','-','9',']' };
				break;
			case 'f': // control symbol in ascii
				toks = { '\f' };
				break;
			case 'n':
				toks = { '\n' };
				break;
			case 'r':
				toks = { '\r' };
				break;
			case 't':
				toks = { '\t' };
				break;
			case 'v':
				toks = { '\v' };
				break;
			case 's': // \s in common regex
				toks = { '[',' ','\f','\n','\r','\t','\v',']' };
				break;
			case 'S': // \S in common regex
				toks = { '[','^',' ','\f','\n','\r','\t','\v',']' };
				break;
			case 'w': // \w in common regex
				toks = { '[','A','-','Z','a','-','z','0','-','9','_',']' };
				break;
			case 'W': // \W in common regex
				toks = { '[','^','A','-','Z','a','-','z','0','-','9','_',']' };
				break;
			default: // escape regex char
				toks.emplace_back(ch, true);
				break;
			}
			return std::move(toks);
		}
		// convert single character to a token
		static std::vector<regextok> tokenize(const std::string& regex) {
			std::vector<regextok> toks;
			std::vector<regextok> rangetoks;
			
			for (auto i = regex.begin(); i != regex.end(); ++i) { // for each character
				if (*i == '\\') { // is escape char
					auto ts = escapeChar(*(i + 1));
					std::move(ts.begin(), ts.end(), back_inserter(toks));
					++i;
				}
				else if (*i == '.') { // is any char
					std::vector<regextok> ts = { '[','^',']' };
					std::move(ts.begin(), ts.end(), back_inserter(toks));
				}
				else
					toks.emplace_back(*i);
			}

			// process range syntax [...]
			bool rg_begin = false; // begin range flag
			bool isfirst = true; // first char flag(char set process)
			bool negated = false; // negative range flag
			std::bitset<256> charset; // set of ascii
			for (auto i = toks.begin(); i != toks.end(); ++i) {
				if (!rg_begin && *i == '[') { // range begin
					rg_begin = true;
					if (*(i + 1) == '^') { // negative range
						negated = true;
						++i;
					}
					rangetoks.emplace_back('('); // range begin symbol
				}
				else if (rg_begin && *i == ']') { // range end
					rg_begin = false;
					isfirst = true;
					// process last closed range
					if (negated) charset.flip(); // filp set 

					for (int i = 1; i < 256; i++) {
						if (charset[i]) { // find char contained
							// continuous sequence merge
							unsigned char from = i;
							unsigned char to = i;
							for (; i < 256 && charset[i]; i++)
								to = i;
							i--;

							if (isfirst) isfirst = false;
							else { rangetoks.emplace_back('|'); } // a range may contain many sub-range

							rangetoks.emplace_back(from, to);
						}
					}
					charset.reset();
					rangetoks.emplace_back(')'); // range end symbol
				}
				else { // other character
					if (rg_begin) { // in a range
						if (*(i + 1) == '-') { // has a connector
							for (int s = *i; s <= *(i + 2); s++) { // contain sub-range
								charset.set(s);
							}
							++i; ++i;
						}
						else charset.set((unsigned char)*i);  // contain seperated char
					}
					else rangetoks.emplace_back(*i); // common char
				}
			}

			return std::move(rangetoks);
		}
		// convert to postfix expression
		static std::vector<regextok> toPost(const std::vector<regextok>& regex) {
			std::map<unsigned char, int> priority = { // operator priority
				{'*',4},
				{'+',4},
				{'?',4},
				{'&',3},
				{'|',2},
				{'(',1}
			};
			std::vector<regextok> res;
			std::stack<unsigned char> ops;

			auto pushOp = [&priority, &ops, &res](unsigned char op) { // loacal function to process new operator
				unsigned char top = 0;
				auto prio = priority.at(op); // of new optr
				while (!ops.empty())
				{
					top = ops.top(); ops.pop(); // get last operator

					if (priority.at(top) >= prio) // lower
						res.push_back(top); // can calculate
					else // higher
					{
						ops.push(top); // put back
						break;
					}
				}
				ops.push(op); // save the optr
			};
			bool last_char = false; // shows if the last token is character (not optr), closure is regarded as character
			for (auto i = regex.begin(); i != regex.end(); ++i) {
				if ((*i) == '*' || (*i) == '+' || (*i) == '?') { // closure
					last_char = true;
					pushOp(*i);
					continue;
				}
				if ((*i) == '|') { // or
					last_char = false;
					pushOp(*i);
					continue;
				}
				if ((*i) == '(') { // bracket
					if (last_char)
						pushOp('&'); // implicit concat operator
					ops.push(*i);
					last_char = false;
					continue;
				}
				if ((*i) == ')') { // close bracket
					unsigned char op = 0;
					while (!ops.empty()) // calculate all optr and find match
					{
						op = ops.top(); ops.pop();
						if (op == '(') break;

						res.push_back(op);
					}
					if (op != '(') {
						throw std::exception("brackets not matched");
					}
					last_char = true;
					continue;
				}
				if (last_char) {
					pushOp('&'); // add implict concat symbol
				}
				res.push_back(*i);
				last_char = true;
			}

			unsigned char op = 0;
			while (!ops.empty()) // calculate the optr left
			{
				op = ops.top(); ops.pop();
				if (op == '(') throw std::exception("brackets not matched");

				res.push_back(op);
			}
			return std::move(res);
		}

	public:
		// process expression to regex token stream (post fix)
		static std::vector<regextok> ProcessRegex(const std::string& regex) {
			return toPost(tokenize(regex));
		}
	};
}