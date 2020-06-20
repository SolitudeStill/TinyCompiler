#pragma once
#include"IntermediateCode.h"

namespace hscp {
	class Optimizer {
	private:
		const std::vector<Quad>& source;
		std::vector<std::vector<Quad>> divided = { {} };
		std::vector<Quad> result;
		void devide() {
			for (const auto& q : source) {
				if (q.op == "LABEL") {
					divided.push_back({});
				}
				divided.back().push_back(q);
				if (q.op == "IF_F" || q.op == "JMP") {
					divided.push_back({});
				}
			}


		}
		bool isDirectVal(std::string test){
			bool isVal = false;
			try
			{
				std::stold(test);
				isVal = true;
			}
			catch (const std::exception&)
			{

			}
			return isVal;
		}
		void optimize() {
			for (auto& block : divided) {
				std::map<std::string, std::string> equalvalents;

				std::vector<Quad> newBlock;
				for (auto& q : block) {
					if (equalvalents.find(q.addr1) != equalvalents.end()) {
						q.addr1 = equalvalents[q.addr1];
					}
					if (q.addr3 != "_" && equalvalents.find(q.addr2) != equalvalents.end()) {
						q.addr2 = equalvalents[q.addr2];
					}
					if (q.op == "MOVE" ) {
						

						if (q.addr2[0]=='_'&&q.addr2[1]=='t')
						{
							equalvalents[q.addr2] = q.addr1;
							q.addr2 = q.addr1;
						}
						
					}
				}
				for (const auto& q : block) {
					if (q.op == "MOVE" && q.addr1 == q.addr2) continue;

					newBlock.push_back(q);
				}

				block = newBlock;
			}
		}

		std::vector<Quad>& getOptimized() {
			for (const auto& block : divided) {
				for (const auto& q : block) {
					result.push_back(q);
				}
			}

			return result;
		}

		Optimizer(const std::vector<Quad>& quads):source(quads){}

	public:
		static std::vector<Quad> Optimize(std::vector<Quad>& quads) {
			Optimizer o(quads);
			o.devide();
			o.optimize();

			return o.getOptimized();
		}
	};
}