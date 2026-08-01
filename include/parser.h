#pragma once

#include <tuple>
#include <string>
#include <functional>
#include <iostream>
#include <limits.h>

#include "empty.h"
#include "view.h"
#include "match.h"
#include "tag.h"

#include "log.h"

#ifndef DEBUG_PRINT_LEVEL
#define DEBUG_PRINT_LEVEL 0
#endif

#define DECL_DEBUG_TAG(TYPE_NAME, LOG_NAME, IS_RULE) \
	struct TYPE_NAME { \
		static constexpr std::string_view name() { return LOG_NAME; } \
		static constexpr bool isRule() { return IS_RULE; } \
	}

namespace ometa {

DECL_DEBUG_TAG(ANONYMOUS, "(anonymous)", false);
DECL_DEBUG_TAG(TAGGER, "(tagger)", false);

template <typename T>
constexpr bool has_backup_method() {
	return requires(T t) {
		t.backup();
	};
}

inline int globalDebugLevel = 0;

inline int nextLogThreshold = INT_MAX;
inline int ruleLogThreshold = 2;
inline int parserLogThreshold = 3;

template <typename Tag, typename F>
class Parser {

protected:

	const F parseFn;

public:

	using parse_fn_type = F;
	using TTag = Tag;

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called internally by parent parsers
	template <forward_range TSource>
	auto parseOn(View<TSource> src, auto& ctx) const {

		int minLogThreshold = std::min(nextLogThreshold, parserLogThreshold);
		if (Tag::isRule()){
			minLogThreshold = std::min(minLogThreshold, ruleLogThreshold);
		}
		bool doLog = globalDebugLevel >= minLogThreshold;
		nextLogThreshold = INT_MAX;

		if constexpr (has_backup_method<decltype(ctx)>()) {
			auto backup = ctx.backup();
		
			if (doLog){
				log(Tag::name(), LogEvent::enter, src);
			}

			auto result = parseFn(src, ctx);

			if (doLog){
				auto evt = result ? LogEvent::accept : LogEvent::reject;
				log(Tag::name(), evt, src, result->next);
			}
		
			if (!result) {
				ctx.backtrack(backup);
			}

			return result;
		}
		else{
			if (doLog){
				log(Tag::name(), LogEvent::enter, src);
			}

			auto result = parseFn(src, ctx);

			if (doLog){
				auto evt = result ? LogEvent::accept : LogEvent::reject;
				log(Tag::name(), evt, src, result->next);
			}

			return result;
		}
	}

	// to be called from the outside to start the parsing process
	auto parse(const auto& src) const {
		return parse(src, empty);
	}

	template <typename TCtx>
	auto parse(const auto& src, TCtx& ctx) const {
		
		int restoreRuleLogThreshold = ruleLogThreshold;
		int restoreParserLogThreshold = parserLogThreshold;
		
		nextLogThreshold = INT_MAX;
		ruleLogThreshold = 2;
		parserLogThreshold = 3;

		auto result = parseOn(View(src), ctx);

		parserLogThreshold = restoreParserLogThreshold;
		ruleLogThreshold = restoreRuleLogThreshold;
		nextLogThreshold = INT_MAX;

		return unwrap(result);
	}
};

template <typename Tag=ANONYMOUS>
auto parser(auto fn){
	return Parser<Tag, decltype(fn)>(fn);
}

template<typename T>
concept DerivedFromParser = requires {typename T::parse_fn_type;}
	&& std::derived_from<T, Parser<typename T::TTag, typename T::parse_fn_type>>;

template <Tag tag, DerivedFromParser P>
auto tagResult(P child){
	auto parseFn = [child]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			auto result = child.parseOn(src, ctx);
			return (result) ?
				makeMaybeMatch(
					makeTagged<tag>(result->value),
					result->next
					)
				: fail;
		};
	return parser<TAGGER>(parseFn);
}

}
