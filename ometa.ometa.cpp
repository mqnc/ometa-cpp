

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>

namespace o = ometa;
using o::operator""_L;

using Snippet = std::string;
Snippet toSnippet(o::SourceView<std::string> src) {
	return src.copyAs<Snippet>();
}
auto operator""_S(const char* compare, size_t size) {
	return Snippet(compare, size);
}

int main(int argc, char* argv[]) {

	auto identStart = o::range(('A'), ('Z')) | o::range(('a'), ('z')) | "_"_L;
	auto identContinue = identStart | o::range(('0'), ('9'));
	auto identifier = o::capture(identStart > *identContinue) >= toSnippet;

	auto cpp = o::dummy<std::string, Snippet>();

	auto ruleForwardDecl = o::dummy<std::string, Snippet>();
	auto ruleDefinition = o::dummy<std::string, Snippet>();
	auto ruleRedefinition = o::dummy<std::string, Snippet>();

	auto bracedCpp = ~"{"_L > o::ref(cpp) > ~"}"_L;
	LOG(bracedCpp);
	cpp = *(
			  bracedCpp >= [](auto value){return "{"_S + value + "}"_S;}
			  | o::ref(ruleDefinition)
			  | identifier
			  | !"}"_L > o::any() >= toSnippet
			  ) >= o::concat;
	LOG(cpp);
	// // todo: handle //, /**/, \\\n, R"()", "", ''

	auto _ = *(" "_L | "\t"_L | "\n"_L) >= o::constant(" "_S);

	auto any = "."_L >= o::constant(Snippet("o::any()"));
	auto epsilon = "()"_L >= o::constant(Snippet("o::epsilon()"));

	auto character =
		~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\\\"_L)
		| !"\\"_L > ~o::any();
	auto literal = o::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet
		> o::insert("_L"_S) >= o::concat;
	LOG(literal);

	auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp
		>= [](auto value) {
			   return "o::range(("_S
				   + o::pick<0>(value)
				   + "), ("_S
				   + o::pick<1>(value)
				   + "))"_S;
		   };
	LOG(range);

	auto choice = o::dummy<std::string, Snippet>();
	auto parenthesized = ~"("_L > ~_ > o::ref(choice) > ~_ > ~")"_L >=
		[](auto value) { return "("_S + value + ")"_S; };
	auto capture = ~"<"_L > ~_ > o::ref(choice) > ~_ > ~">"_L >=
		[](auto value) { return "o::capture("_S + value + ")"_S; };
	LOG(capture);

	auto action =
		identifier >= toSnippet
		| bracedCpp >= [](auto value) {
			  return "[](auto value){"_S + value + "}"_S;
		  };
	auto predicate = ~"?"_L > ~_ >
		(
			identifier >= toSnippet
			| bracedCpp >= [](auto value) {
					return "[](auto value){"_S + value + "}"_S;
				}
		);

	auto freeActionOrPredicate = ~"^"_L > ~_ > o::insert("(epsilon()"_S)
		> (
			  o::insert(" => "_S) > action
			  | o::insert(" =< "_S) > predicate
			  ) > o::insert(")"_S) >= o::concat;

	auto boundActionOrPredicate = ~"->"_L > ~_
		> (
			  o::insert(" >= "_S) > action
			  | o::insert(" <= "_S) > predicate
			  ) >= o::concat;

	auto primary =
		identifier
		| any
		| epsilon
		| literal
		| range
		| capture
		| freeActionOrPredicate
		| parenthesized;

	auto repetition = primary >
		-(
			"?"_L >= o::constant("-"_S)
			| "*"_L >= o::constant("*"_S)
			| "+"_L >= o::constant("+"_S)
			)
		>= [](auto value) {
			   if (pick<1>(value).size() == 0) {
				   return pick<0>(value);
			   }
			   else {
				   return pick<1>(value)[0] + pick<0>(value);
			   }
		   };
	auto lookAhead = o::capture(-("&"_L | "!"_L)) >= toSnippet
		> ~_ > repetition >= o::concat;
	auto sequence = lookAhead >
		*((~_ > (
					o::insert(" > "_S) > lookAhead >= o::concat
					| boundActionOrPredicate
					)) >= o::concat) >= o::concat;
	LOG(sequence);
	choice = sequence > *(
							~_ > "|"_L >= o::constant(" | "_S) > ~_ > sequence >= o::concat
							) >= o::concat;
	LOG(choice);

	ruleForwardDecl = ~"("_L > ~_ > identifier > ~_ > ~")"_L > ~_ > ~":="_L > ~_
		> bracedCpp > ~_ > ~"->"_L > ~_ > bracedCpp > ~_ > ~";"_L
		>= [](auto value) {
			   return "auto "_S + pick<0>(value) + " = o::dummy<("
				   + pick<1>(value) + "), (" + pick<2>(value) + ")>();";
		   };

	ruleDefinition = identifier > ~_ > ~":="_L > ~_ > choice > ~_ > ~";"_L
		>= [](auto value) { return "auto "_S + pick<0>(value) + " = "_S + pick<1>(value) + ";"; };

	ruleRedefinition = identifier > ~_ > ~":>"_L > ~_ > choice > ~_ > ~";"_L
		>= [](auto value) { return pick<0>(value) + " = "_S + pick<1>(value) + ";"; };

	auto code = o::readFile("../ometa.ometa");

	auto result = cpp.parse(code);
	if (result) {
		o::writeFile("../ometa.ometa.cpp", *result);
	}
	else {
		std::cout << "fail\n";
	}

	return EXIT_SUCCESS;
}
