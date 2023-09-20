#pragma once

#include <tuple>
#include <string>
#include <functional>
#include <iostream>

#include "empty.h"
#include "sourceview.h"
#include "match.h"
#include "tag.h"

#ifndef NDEBUG
#	include "debug.h"
#endif

namespace ometa {

template <typename F>
class Parser {
public:
	F parseFn;

#ifndef NDEBUG
	std::string name = "";
#endif

	Parser(F parseFn): parseFn {parseFn} {}

	// to be called internally by parent parsers
	template <forward_range TSource>
	auto parseOn(SourceView<TSource> src, auto ctx) const {
#ifdef NDEBUG
		return parseFn(src, ctx);
#else
		if (name == "") {
			return parseFn(src, ctx);
		}
		else {
			log(name, LogEvent::enter, src);
			auto result = parseFn(src, ctx);
			auto evt = result ? LogEvent::accept : LogEvent::reject;
			log(name, evt, src, result->next);
			return result;
		}
#endif
	}

	// to be called from the outside to start the parsing process
	template <typename TCtx = Empty>
	auto parse(const auto& src, TCtx ctx = empty) const {
		auto result = parseOn(SourceView(src), ctx);
		return unwrap(result);
	}

	template <typename F2>
	void operator=(const Parser<F2>& target) {
		parseFn = target.parseFn;
	}

	template <Tag tag>
	auto as() {

		auto parseFn = [this]<forward_range TSource>
			(
				SourceView<TSource> src,
				auto ctx
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

#ifdef NDEBUG
	Parser operator[](std::string) { return *this; }
#else
	auto operator[](std::string name) {
		auto parseFn = [this]<forward_range TSource>(
						   SourceView<TSource> src, auto ctx
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
