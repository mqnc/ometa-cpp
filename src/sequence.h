#pragma once

#include "parser.h"
#include "stringliteral.h"

template<typename... TChildren>
auto makeSequence(TChildren... children) {

	auto parseFn = []<forward_range TSource>(
		SourceView<TSource> src,
		auto children,
		auto ctx
		) {
		using tuple_type = std::tuple<typename decltype(std::declval<TChildren>().parse(src, ctx))::value_type...>;

		tuple_type matches; // screw RAII
		auto next = src;
		bool success = true;

		auto recursiveSteps = [&]<size_t I = 0>(const auto & self, auto ctx){
			if constexpr (I < sizeof...(TChildren)) {
				auto result = std::get<I>(children).parse(next, ctx);
				if(result.has_value()){
					std::get<I>(matches) = result.value();
					next = result->next;
					auto newCtx = []<forward_range TSrc, typename TVal, StringLiteral Tag>(Match<TSrc, TVal, Tag> match, auto ctx){
						if constexpr(Tag == StringLiteral("")){
							return ctx;
						}
						else{
							return ctx.template add<Tag>(match);
						}
					}(result.value(), ctx);
					self.template operator() <I+1> (self, newCtx);
				}
				else{
					success = false;
				}
			}
		};

		recursiveSteps(recursiveSteps, ctx);

		return success? match(src, next, matches) : fail;
	};

	return Parser(parseFn, std::make_tuple(children...));
}

// the PartialSequence wrapper allows chains like
// a > b > c with the parsing result being
// tuple<ValA, ValB, ValC> instead of
// tuple<tuple<ValA, ValB>, ValC>
template <typename F, typename TChildren = std::tuple<>>
class PartialSequence : public Parser<F, TChildren> {
public: PartialSequence(Parser<F, TChildren>&& other) :
	Parser<F, TChildren>(std::move(other)) {};
};

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator> (Parser<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	return PartialSequence{ makeSequence(parser1, parser2) };
}

template <typename F1, typename TChildren1, typename F2, typename TChildren2>
auto operator> (PartialSequence<F1, TChildren1> parser1, Parser<F2, TChildren2> parser2) {
	auto allChildren = std::tuple_cat(parser1.children, std::make_tuple(parser2));
	return PartialSequence{
		std::apply([](auto &&... args) {
			return makeSequence(args...);
		}, allChildren)
	};
}