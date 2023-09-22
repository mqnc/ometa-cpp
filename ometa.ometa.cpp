

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

namespace o = ometa;
using o::operator""_L;
using o::concat;

using Snippet = std::string;
Snippet toSnippet(o::SourceView<std::string> src) {
	return src.copyAs<Snippet>();
}
auto operator""_S(const char* compare, size_t size) {
	return Snippet(compare, size);
}

int main(int argc, char* argv[]) {

	auto _ = o::capture(*(" "_L | "\t"_L | "\n"_L)) >= toSnippet;

	auto identStart = o::range(('A'), ('Z')) | o::range(('a'), ('z')) | "_"_L;
	auto identContinue = identStart | o::range(('0'), ('9'));
	auto identifier = o::capture(identStart > *identContinue) >= toSnippet;
	auto reference = ~"@"_L > identifier >= [](auto value){return "o::ref("_S + value + ")"_S;};

	auto cppChar = ~"\\"_L > ~o::any() | ~o::any();
	auto cppLiteral = o::capture(~"'"_L > !"'"_L > ~cppChar > ~"'"_L
		| ~"\""_L > ~*(!"\""_L > ~cppChar) > ~"\""_L) >= toSnippet;

	auto valueRef = ~"$"_L >= [](auto value){return "value"_S;};
	auto indexedValueRef = ~"$"_L > o::capture(+o::range(('0'), ('9'))) >= toSnippet >= [](auto value){return "o::pick<"_S + value + "-1>(value)"_S;};

	auto cpp = o::dummy<std::string, Snippet>();

	auto ruleForwardDecl = o::dummy<std::string, Snippet>();
	auto ruleDefinition = o::dummy<std::string, Snippet>();
	auto ruleRedefinition = o::dummy<std::string, Snippet>();

	auto bracedCpp = ~"{"_L > o::ref(cpp) > ~"}"_L;
	
	cpp = *(o::ref(ruleForwardDecl)
		| o::ref(ruleDefinition)
		| o::ref(ruleRedefinition)
		| identifier
		| cppLiteral
		| bracedCpp >= [](auto value){ return "{"_S + value + "}"_S; }
		| indexedValueRef
		| valueRef
		| !"}"_L > o::any() >= toSnippet) >= concat;

	auto any = "."_L >= [](auto value){return "o::any()"_S;};
	auto epsilon = "()"_L >= [](auto value){return "o::epsilon()"_S;};

	auto character = ~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\"_L)
		| !"\\"_L > ~o::any();
	auto literal = o::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet > (o::epsilon() >= [](auto value){return "_L"_S;}) >= concat;

	auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp >= [](auto value){
		return "o::range(("_S + o::pick<1-1>(value) + "), ("_S + o::pick<2-1>(value) + "))"_S;
	};

	auto choice = o::dummy<std::string, Snippet>();
	auto parenthesized = ~"("_L > ~_ > o::ref(choice) > ~_ > ~")"_L >= [](auto value){ return "("_S + value + ")"_S; };
	auto capture = ~"<"_L > ~_ > o::ref(choice) > ~_ > ~">"_L >= [](auto value){ return "o::capture("_S + value + ")"_S; };

	auto action = identifier >= toSnippet
		| bracedCpp >= [](auto value){ return "[](auto value){"_S + value + "}"_S; };
	auto predicate = ~"?"_L > ~_ > (identifier >= toSnippet
		| bracedCpp >= [](auto value){ return "[](auto value){"_S + value + "}"_S; });

	auto freeActionOrPredicate = ~"^"_L > ~_ > (o::epsilon() >= [](auto value){ return "(o::epsilon()"_S; }) > ((o::epsilon() >= [](auto value){return " >= "_S;}) > action
		| (o::epsilon() >= [](auto value){return " <= "_S;}) > predicate) > (o::epsilon() >= [](auto value){return ")"_S;}) >= concat;

	auto boundActionOrPredicate = ~"->"_L > ~_ > ((o::epsilon() >= [](auto value){return " >= "_S;}) > action
		| (o::epsilon() >= [](auto value){return " <= "_S;}) > predicate) >= concat;

	auto primary = identifier
		| reference
		| any
		| epsilon
		| literal
		| range
		| capture
		| freeActionOrPredicate
		| parenthesized;

	auto postfix = primary > -("?"_L >= [](auto value){return "-"_S;}
		| "*"_L >= [](auto value){return "*"_S;}
		| "+"_L >= [](auto value){return "+"_S;}) >= [](auto value){
		if (o::pick<2-1>(value).size() == 0) {
			return o::pick<1-1>(value);
		}
		else {
			return o::pick<2-1>(value)[0] + o::pick<1-1>(value);
		}
	};

	auto prefix = o::capture(-("&"_L | "!"_L | "~"_L)) >= toSnippet > ~_ > postfix >= concat;

	auto sequence = prefix > *((~_ > ((o::epsilon() >= [](auto value){return " > "_S;}) > prefix >= concat
			| boundActionOrPredicate)) >= concat) >= concat;

	choice = sequence > *(o::capture(_ > "|"_L > _) >= toSnippet > sequence >= concat) >= concat;

	ruleForwardDecl = ~"("_L > ~_ > identifier > ~_ > ~")"_L > ~_ > ~":="_L > ~_ > bracedCpp > ~_ > ~"->"_L > ~_ > bracedCpp > ~_ > ~";"_L >= [](auto value){
			return "auto "_S + o::pick<1-1>(value) + " = o::dummy<"
				+ o::pick<2-1>(value) + ", " + o::pick<3-1>(value) + ">();";
		};

	ruleDefinition = identifier > ~_ > ~":="_L > ~_ > choice > ~_ > ~";"_L >= [](auto value){ return "auto "_S + o::pick<1-1>(value) + " = "_S + o::pick<2-1>(value) + ";"; };

	ruleRedefinition = identifier > ~_ > ~":>"_L > ~_ > choice > ~_ > ~";"_L >= [](auto value){ return o::pick<1-1>(value) + " = "_S + o::pick<2-1>(value) + ";"; };




	auto code = o::readFile("../ometa.ometa");

	auto result = cpp.parse(code);
	if (result) {
		try {
			auto backup = o::readFile("../ometa.ometa.cpp");
			std::time_t time = std::time({});
			char timeString[std::size("yyyy_mm_dd__hh_mm_ssZ")];
			std::strftime(std::data(timeString), std::size(timeString),
				"%Y_%m_%d__%H_%M_%S", std::gmtime(&time));
			o::writeFile(std::string("../ometa.ometa.cpp.") + timeString + ".backup", backup);
		}
		catch (...) {}

		o::writeFile("../ometa.ometa.cpp", *result);
	}
	else {
		std::cout << "fail\n";
	}

	return EXIT_SUCCESS;
}
