#pragma once
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>
#include<sstream>
#include<functional>

namespace hscp {
	enum class title_type // types of regex in config
	{
		reserve, symbol, structure
	};
	struct token_define // token in config
	{
		title_type type;
		std::string id;
		std::string expr;
		int line;
		bool ref;
	};
	struct error // error info
	{
		int line;
		std::string what;
	};
	// load lexical rules from file and restore
	class FileLoader {
	private:
		std::vector<token_define> tokens;
		std::vector<error> errors;
		std::string reg_reserve = "&|*.+[-]^?()`\\"; // symbols used to control regex

		std::map<std::string, title_type> title = { // string to title type
			{"[reserve]", title_type::reserve},
			{"[sign]", title_type::symbol},
			{"[structure]", title_type::structure}
		};
		// read a line, return if it is a type (not regex), and pass out the type if true
		bool is_title(const std::string& line, title_type& type) {
			if (title.find(line) != title.end()) {
				type = title[line];
				return true;
			}
			return false;
		}
		// add escape before characters act as control character (only in [symbol])
		void pre_escape() {
			for (auto& e : tokens) {
				if (e.type != title_type::symbol) continue;

				for (size_t i = 0; i < e.expr.size(); i++) { // search expression
					if (reg_reserve.find(e.expr[i]) != std::string::npos) { // in reserved symbols
						e.expr.insert(i, "\\"); // insert escape char
						i++;
					}
				}
			}
		}
		// replace referrence (quoted by '`') with referred expression
		void dereferrence() {
			for (auto& i : tokens) {
				// reference syntax only exist in [structure] field
				if (i.type != title_type::structure) continue;

				size_t offset = 0;
				size_t refsign_pos = i.expr.find('`'); // refer symbol
				while (refsign_pos != std::string::npos) { // read all ref symbol
					if (refsign_pos == 0 || i.expr[refsign_pos - 1] != '\\') { // this is not escaped ref symbol
						size_t right_pos = i.expr.find('`', refsign_pos + 1); // close symbol
						if (right_pos != std::string::npos && i.expr[right_pos - 1] != '\\') // not escaped
						{
							std::string ref_key = i.expr.substr(refsign_pos + 1, right_pos - refsign_pos - 1); // get key
							auto it = find_if(tokens.begin(), tokens.end(), [ref_key](const token_define& e) {return e.id == ref_key; }); // referred expression
							if (it != tokens.end()) {
								i.expr.replace(refsign_pos, right_pos - refsign_pos + 1, "(" + (*it).expr + ")"); // copy expression
							}
							else {
								errors.push_back({ i.line, "referrence key not found" }); // rules has error
								offset = refsign_pos + 1;
							}
						}
						else {
							errors.push_back({ i.line, "invalid referrence symbol" }); // single ref symbol error
							offset = refsign_pos + 1;
						}
					}
					else {
						offset = refsign_pos + 1;
					}
					refsign_pos = i.expr.find('`', offset); // find next
				}
			}
		}
		// load lexture file
		void load_lex(const std::string& def_file) {
			std::ifstream ifs(def_file, std::ios::in | std::ios::_Nocreate); // open file
			std::string line;
			int count = 0; // lines counter
			title_type type;
			while (std::getline(ifs, line)) // read lines
			{
				count++;
				if (!is_title(line, type)) { // ignore title line 
					std::stringstream t(line);
					std::string k, v;
					std::getline(t, k, ' '); // read 'name' for expression
					t >> std::ws; // eat the space
					std::getline(t, v); // read expression
					if (k[0] == '^') {
						tokens.push_back({ type,k.substr(1),v,count,true }); // this is referrence item, which can be referred and will be delete afterwards
					}
					else
						tokens.push_back({ type,k,v,count,false }); // this is expression item
				}
			}
			// escape symbols in [sign] field
			pre_escape();
			// move referred item to its referrer
			dereferrence();
		}
		// remove referrence only
		void rmref() {
			auto it = tokens.begin();
			while ((it = find_if(it, tokens.end(), [](const auto& t) {return t.ref; })) != tokens.end()) {
				it--;
				tokens.erase(it + 1);
			}
		}

	public:
		// load file then pass errors and regex
		FileLoader(const std::string& def_file, std::function<void(const std::vector<error>&)> report, std::function<void(const std::vector<token_define>&)> next) {
			// load file
			load_lex(def_file);

			// remove referrence only expression
			rmref();

			report(errors);
			next(tokens);
		}
		// check if error exists
		bool NoError() const {
			return errors.size() == 0;
		}
		// pass out tokens read
		const std::vector<token_define>& LexTokens() const {
			return tokens;
		}
	};
}