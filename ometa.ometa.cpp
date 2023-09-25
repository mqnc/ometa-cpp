

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

	auto tStart = tic();

	const auto _ = o::capture(*(" "_L | "\t"_L | "\n"_L)) >= toSnippet;

	const auto identStart = o::range(('A'), ('Z')) | o::range(('a'), ('z')) | "_"_L;
	const auto identContinue = identStart | o::range(('0'), ('9'));
	const auto identifier = o::capture(identStart > *identContinue) >= toSnippet;
	const auto reference = identifier > ~"'"_L >= [](auto value){return "o::ptr("_S + value + ")"_S;};

	const auto cppChar = ~"\\"_L > ~o::any() | ~o::any();
	const auto cppLiteral = o::capture(~"'"_L > !"'"_L > ~cppChar > ~"'"_L
		| ~"\""_L > ~*(!"\""_L > ~cppChar) > ~"\""_L) >= toSnippet;

	const auto valueRef = ~"$"_L >= [](auto value){return "value"_S;};
	const auto indexedValueRef = ~"$"_L > o::capture(+o::range(('0'), ('9'))) >= toSnippet >= [](auto value){return "o::pick<"_S + value + "-1>(value)"_S;};

	auto cpp = o::declare<std::string, Snippet>();

	auto ruleForwardDecl = o::declare<std::string, Snippet>();
	auto ruleDefinition = o::declare<std::string, Snippet>();
	auto ruleRedefinition = o::declare<std::string, Snippet>();

	const auto bracedCpp = ~"{"_L > o::ptr(cpp) > ~"}"_L;
	
	*cpp = *(o::ptr(ruleForwardDecl)
		| o::ptr(ruleDefinition)
		| o::ptr(ruleRedefinition)
		| identifier
		| cppLiteral
		| bracedCpp >= [](auto value){ return "{"_S + value + "}"_S; }
		| indexedValueRef
		| valueRef
		| !"}"_L > o::any() >= toSnippet) >= concat;

	const auto any = "."_L >= [](auto value){return "o::any()"_S;};
	const auto epsilon = "()"_L >= [](auto value){return "o::epsilon()"_S;};

	const auto character = ~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\"_L)
		| !"\\"_L > ~o::any();
	const auto literal = o::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet > (o::epsilon() >= [](auto value){return "_L"_S;}) >= concat;

	const auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp >= [](auto value){
		return "o::range(("_S + o::pick<1-1>(value) + "), ("_S + o::pick<2-1>(value) + "))"_S;
	};

	auto choice = o::declare<std::string, Snippet>();
	const auto parenthesized = ~"("_L > ~_ > o::ptr(choice) > ~_ > ~")"_L >= [](auto value){ return "("_S + value + ")"_S; };
	const auto capture = ~"<"_L > ~_ > o::ptr(choice) > ~_ > ~">"_L >= [](auto value){ return "o::capture("_S + value + ")"_S; };

	const auto action = identifier >= toSnippet
		| bracedCpp >= [](auto value){ return "[](auto value){"_S + value + "}"_S; };
	const auto predicate = ~"?"_L > ~_ > (identifier >= toSnippet
		| bracedCpp >= [](auto value){ return "[](auto value){"_S + value + "}"_S; });

	const auto freeActionOrPredicate = ~"^"_L > ~_ > (o::epsilon() >= [](auto value){ return "(o::epsilon()"_S; }) > ((o::epsilon() >= [](auto value){return " >= "_S;}) > action
		| (o::epsilon() >= [](auto value){return " <= "_S;}) > predicate) > (o::epsilon() >= [](auto value){return ")"_S;}) >= concat;

	const auto boundActionOrPredicate = ~"->"_L > ~_ > ((o::epsilon() >= [](auto value){return " >= "_S;}) > action
		| (o::epsilon() >= [](auto value){return " <= "_S;}) > predicate) >= concat;

	const auto primary = reference
		| identifier
		| any
		| epsilon
		| literal
		| range
		| capture
		| freeActionOrPredicate
		| parenthesized;

	const auto postfix = primary > -("?"_L >= [](auto value){return "-"_S;}
		| "*"_L >= [](auto value){return "*"_S;}
		| "+"_L >= [](auto value){return "+"_S;}) >= [](auto value){
		if (o::pick<2-1>(value).size() == 0) {
			return o::pick<1-1>(value);
		}
		else {
			return o::pick<2-1>(value)[0] + o::pick<1-1>(value);
		}
	};

	const auto prefix = o::capture(-("&"_L | "!"_L | "~"_L)) >= toSnippet > ~_ > postfix >= concat;

	const auto sequence = prefix > *((~_ > ((o::epsilon() >= [](auto value){return " > "_S;}) > prefix >= concat
			| boundActionOrPredicate)) >= concat) >= concat;

	*choice = sequence > *(o::capture(_ > "|"_L > _) >= toSnippet > sequence >= concat) >= concat;

	*ruleForwardDecl = identifier > ~"'"_L > ~_ > ~":"_L > ~_ > bracedCpp > ~_ > ~"->"_L > ~_ > bracedCpp > ~_ > ~";"_L >= [](auto value){
			return "auto "_S + o::pick<1-1>(value) + " = o::declare<"
				+ o::pick<2-1>(value) + ", " + o::pick<3-1>(value) + ">();";
		};

	*ruleDefinition = identifier > ~_ > ~":="_L > ~_ > o::ptr(choice) > ~_ > ~";"_L >= [](auto value){ return "const auto "_S + o::pick<1-1>(value) + " = "_S + o::pick<2-1>(value) + ";"; };

	*ruleRedefinition = identifier > ~"'"_L > ~_ > ~"=>"_L > ~_ > o::ptr(choice) > ~_ > ~";"_L >= [](auto value){ return "*"_S + o::pick<1-1>(value) + " = "_S + o::pick<2-1>(value) + ";"; };

	std::cout << "setup: ";
	toc(tStart);


	auto code = o::readFile("../ometa.ometa");

	auto result = cpp->parse(code);
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
