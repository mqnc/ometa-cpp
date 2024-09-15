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
public:
	F parseFn;

#ifdef DEBUG_PRINTS
	mutable std::string name = "";
#endif

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called internally by parent parsers
	template <forward_range TSource>
	auto parseOn(View<TSource> src, auto& ctx) const {

#ifdef DEBUG_PRINTS
		if (name != "") {
			log(name, LogEvent::enter, src);
		}
#endif

		if constexpr (has_backup_method<decltype(ctx)>()) {
			auto backup = ctx.backup();
			auto result = parseFn(src, ctx);
			if (!result) {
				ctx.backtrack(backup);
			}
#ifdef DEBUG_PRINTS
			if (name != "") {
				auto evt = result ? LogEvent::accept : LogEvent::reject;
				log(name, evt, src, result->next);
			}
#endif
			return result;
		}
		else {
#ifdef DEBUG_PRINTS
			if (name != "") {
				auto result = parseFn(src, ctx);
				auto evt = result ? LogEvent::accept : LogEvent::reject;
				log(name, evt, src, result->next);
				return result;
			}
			else {
				return parseFn(src, ctx);
			}
#else
			return parseFn(src, ctx);
#endif
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

	template <typename F2>
	void operator=(const Parser<F2>& target) {
		parseFn = target.parseFn;
	}

	template <Tag tag>
	auto as() const {

		auto parseFn = [this]<forward_range TSource>
			(
				View<TSource> src,
				auto& ctx
			) {
				auto result = this->parseOn(src, ctx);
				return (result) ?
					makeMaybeMatch(
						makeTagged<tag>(result->value),
						result->next
						)
					: fail;
			};

		return Parser<decltype(parseFn)>(parseFn);
	}

#ifndef DEBUG_PRINTS
	Parser operator[](std::string) { return *this; }
#else
	auto operator[](std::string name) {
		auto parseFn = [this]<forward_range TSource>(
						   View<TSource> src, const auto& ctx
					   ) {
			return this->parseOn(src, ctx);
		};

		auto wrap = Parser<decltype(parseFn)>(parseFn);
		wrap.name = name;
		return wrap;
	}
#endif

	// template <typename TSource>
	// using TValue = decltype(std::declval<Parser<F, TChildren>>().parse(std::declval<TSource>()));

};

}
