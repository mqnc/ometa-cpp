#pragma once

#include <tuple>
#include <string>
#include <functional>
#include <iostream>

#include "empty.h"
#include "view.h"
#include "match.h"
#include "tag.h"

#include "debug.h"

namespace ometa {

template <typename T>
constexpr bool has_backup_method() {
	return requires(T t) {
		t.backup();
	};
}

template <typename F>
class Parser {
protected:
	F parseFn;
public:
	using parse_fn_type = F;

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called internally by parent parsers
	template <forward_range TSource>
	auto parseOn(View<TSource> src, auto& ctx) const {
		if constexpr (has_backup_method<decltype(ctx)>()) {
			auto backup = ctx.backup();
			auto result = parseFn(src, ctx);
			if (!result) {
				ctx.backtrack(backup);
			}
			return result;
		}
		else {
			return parseFn(src, ctx);
		}
	}

	// to be called from the outside to start the parsing process
	auto parse(const auto& src) const {
		auto result = parseOn(View(src), empty);
		return unwrap(result);
	}

	template <typename TCtx>
	auto parse(const auto& src, TCtx& ctx) const {
		auto result = parseOn(View(src), ctx);
		return unwrap(result);
	}

	const F& getParseFn() const {
		return parseFn;
	}
};

template<typename T>
concept DerivedFromParser = requires {typename T::parse_fn_type;}
	&& std::derived_from<T, Parser<typename T::parse_fn_type>>;

template <Tag tag, DerivedFromParser P>
auto tagResult(P parser){
	auto parseFn = [parser]<forward_range TSource>
		(
			View<TSource> src,
			auto& ctx
		) {
			auto result = parser.parseOn(src, ctx);
			return (result) ?
				makeMaybeMatch(
					makeTagged<tag>(result->value),
					result->next
					)
				: fail;
		};

	return Parser(parseFn);
}

}
