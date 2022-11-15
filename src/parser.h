#pragma once

#include <tuple>
#include "empty.h"
#include "sourceview.h"
#include "match.h"

template <typename F, typename TChildren = std::tuple<>>
class Parser {
protected:
	F parseFn;
public:

	TChildren children;

	Parser(F parseFn, TChildren children = std::tuple<>{}) :
		parseFn{ parseFn }, children{ children } {}

	auto parse(auto src) {
		if constexpr (std::is_same_v<decltype(src), const char*>) {
			return parseFn(SourceView(std::string(src)), children, Empty{});
		}
		else {
			return parseFn(SourceView(src), children, Empty{});
		}
	}

	auto parse(auto src, auto ctx) {
		if constexpr (std::is_same_v<decltype(src), const char*>) {
			return parseFn(SourceView(std::string(src)), children, ctx);
		}
		else {
			return parseFn(SourceView(src), children, ctx);
		}
	}

	template<forward_range TSource>
	auto parse(SourceView<TSource> src, auto ctx) {
		return parseFn(src, children, ctx);
	}

	//template <typename TSource>
	//using TValue = decltype(std::declval<Parser>().parse(std::declval<TSource>()));

};
