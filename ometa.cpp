

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
	auto bracedCpp = ~"{"_L > o::ref(cpp) > ~"}"_L;
	cpp = o::capture(
			  ~*(
				  ~bracedCpp
				  | !"}"_L > ~o::any()
				  )
			  ) >= toSnippet;
	// // todo: handle //, /**/, \\\n, R"()", "", ''

	auto _ = *(" "_L | "\t"_L | "\n"_L) >= o::constant(" "_S);

	auto identStart = o::range('A', 'Z') | o::range('a', 'z') | "_"_L;
	auto identContinue = identStart | o::range('0', '9');
	auto identifier = o::capture(identStart > *identContinue) >= toSnippet;

	auto any = "."_L >= o::constant(Snippet("o::any()"));
	auto epsilon = "()"_L >= o::constant(Snippet("o::epsilon()"));

	auto character =
		~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\\\"_L)
		| !"\\"_L > ~o::any();
	auto literal = o::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet
		> o::insert("_L"_S) >= o::concat;

	auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp
		>= [](auto value) {
			   return "o::range(("_S
				   + o::pick<0>(value)
				   + "), ("_S
				   + o::pick<1>(value)
				   + "))"_S;
		   };

	auto choice = o::dummy<std::string_view, Snippet>();
	auto parenthesized = ~"("_L > ~_ > o::ref(choice) > ~_ > ~")"_L >=
		[](auto value) { return "("_S + value + ")"_S; };
	auto capture = ~"<"_L > ~_ > o::ref(choice) > ~_ > ~">"_L >=
		[](auto value) { return "o::capture("_S + value + ")"_S; };

	auto action = bracedCpp >= [](auto value) {
		return "[](auto value){"_S + value + "}"_S;
	};
	auto predicate = bracedCpp > ~_ > ~"?"_L >= [](auto value) {
		return "[](auto value){"_S + value + "}"_S;
	}; ;

	auto primary =
		identifier
		| any
		| epsilon
		| literal
		| range
		| capture
		| o::insert("(epsilon() <= "_S) > predicate > o::insert(")"_S) >= o::concat
		| o::insert("(epsilon() >= "_S) > action > o::insert(")"_S) >= o::concat
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
		*((~_ > o::insert(" > "_S) >
			  (
				  lookAhead
				  | ~"->"_L > ~_ > o::insert("<="_S) > predicate >= o::concat
				  | ~"->"_L > ~_ > o::insert(">="_S) > action >= o::concat
				  )) >= o::concat) >= o::concat;
	choice = sequence > ~_ > *(
								 "|"_L >= o::constant(" | "_S) > ~_ > sequence >= o::concat
								 ) >= o::concat;
	auto rule = identifier["id"] > ~_ > ~":="_L > ~_ > choice["cho"] > ~_ > ~";"_L
		>= [](auto value) { return "auto "_S + pick<0>(value) + " = "_S + pick<1>(value) + ";\n"; };

	auto result = rule.parse("x := <a (b|c)?> -> {awa;};");
	if (result) {
		std::cout << "success: »" << *result << "«\n";
	}
	else {
		std::cout << "fail\n";
	}

	return EXIT_SUCCESS;
}
