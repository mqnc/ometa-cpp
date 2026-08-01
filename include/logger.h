#pragma once

#include "parser.h"

namespace ometa {

DECL_DEBUG_TAG(LOGGER, "(logger)", false);

template <typename F>
class LoggerWrapper: public ParserWrapper<LOGGER, F>{
public:
	std::string getLevel() const {return this->parseFn.lvl;}
	using ParserWrapper<LOGGER, F>::ParserWrapper;
};

template <DerivedFromParser T>
class LoggerFn {

	int setNextLogThreshold = -1;
	int setRuleLogThreshold = -1;
	int setParserLogThreshold = -1;

public:
	const std::string lvl;
	const T child;

	LoggerFn(std::string level, T parser):
		lvl{level},
		child{parser}
	{
		for (size_t i = 0; i < level.size(); ++i) {
			int value = static_cast<int>(level[i] - '0');
			switch (i) {
				case 0: setNextLogThreshold = value; break;
				case 1: setRuleLogThreshold = value; break;
				case 2: setParserLogThreshold = value; break;
			}
		}
	};
	
	auto operator()(auto src, auto& ctx) const {
		int restoreRuleLogThreshold = ruleLogThreshold;
		int restoreParserLogThreshold = parserLogThreshold;

		if(setNextLogThreshold != -1){nextLogThreshold = setNextLogThreshold;}
		if(setRuleLogThreshold != -1){ruleLogThreshold = setRuleLogThreshold;}
		if(setParserLogThreshold != -1){parserLogThreshold = setParserLogThreshold;}

		auto result = child.parseOn(src, ctx);

		parserLogThreshold = restoreParserLogThreshold;
		ruleLogThreshold = restoreRuleLogThreshold;

		return result;
	}
};

template <DerivedFromParser T>
auto logger(std::string level, T child) {
	return LoggerWrapper<decltype(LoggerFn(level, child))>(LoggerFn(level, child));
}

}
