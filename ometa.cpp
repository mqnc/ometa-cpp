

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <string>

namespace o = ometa;
using o::operator""_L;

using Snippet = std::string;
Snippet toSnippet(o::SourceView<std::string_view> src) {
	return src.copyAs<Snippet>();
}
auto operator""_S(const char* compare, size_t size) {
	return Snippet(compare, size);
}

int main(int argc, char* argv[]) {

	auto cpp = o::dummy<std::string_view, Snippet>();
	auto bracedCpp = (~"{"_L["{"] > o::ref(cpp)["^cpp"] > ~"}"_L["}"])["{cpp}"];
	cpp = (bracedCpp["^{cpp}"]
		| o::capture(*((!"}"_L)["!}"] > o::any()["."])) >= toSnippet)["cpp"];
	// // todo: handle //, /**/, \\\n, R"()", "", ''

	auto _ = *(" "_L | "\t"_L | "\n"_L) >= o::constant(" "_S);

	auto identStart = o::range('A', 'Z') | o::range('a', 'z') | "_"_L;
	auto identContinue = identStart | o::range('0', '9');
	auto identifier = o::capture(identStart > *identContinue) >= toSnippet;

	auto any = o::capture("."_L) >= o::constant(Snippet("o::any()"));
	auto epsilon = "()"_L >= o::constant(Snippet("o::epsilon()"));

	auto character =
		~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\\\"_L)
		| !"\\"_L > ~o::any();
	auto literal = o::capture("\""_L > *(!"\""_L > character) > "\""_L)
		> o::insert("_L"_S)
		>= [](auto value) -> Snippet {
		return o::pick<0>(value).template copyAs<Snippet>()
			+ o::pick<1>(value);
	};
	auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp
		>= [](auto value) {
			   return "o::range(("_S
				   + o::pick<0>(value)
				   + "), ("
				   + o::pick<1>(value)
				   + "))";
		   };

	// auto choice = o::dummy<std::string_view, o::Empty>();
	// auto parenthesized = "("_L > _ > o::ref(choice) > _ > ")"_L;
	// auto capture = "<"_L > _ > o::ref(choice) > _ > ">"_L;

	// auto action = bracedCpp;
	// auto predicate = bracedCpp > _ > "?"_L;

	// auto primary = identifier
	// 	| o::capture(any)
	// 	| o::capture(epsilon)
	// 	| o::capture(literal)
	// 	| o::capture(range)
	// 	| o::capture(capture)
	// 	| o::capture(action)
	// 	| o::capture(predicate)
	// 	| o::capture(parenthesized);

	// auto repetition = primary > -("?"_L | "*"_L | "+"_L);
	// auto lookAhead = ("&"_L | "!"_L) > _ > repetition;
	// auto sequence = lookAhead > *(_ > (
	// 									  lookAhead >= o::constant(o::empty)
	// 									  | "->"_L > _ > action >= o::constant(o::empty)
	// 									  | "->"_L > _ > predicate >= o::constant(o::empty)
	// 									  ));
	// choice = sequence > _ > *("|"_L > _ > sequence) >= o::constant(o::empty);
	// auto rule = identifier > _ > ":="_L > choice > _ > ";"_L;

	auto result = bracedCpp.parse("{  a\te\n}");
	if (result) {
		std::cout << "success: «" << *result << "»\n";
	}
	else {
		std::cout << "fail\n";
	}

	return EXIT_SUCCESS;
}
