

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

using ometa::operator""_L;

using Snippet = std::string;
auto toSnippet = ometa::action([](auto src) {
	if constexpr(std::is_same_v<decltype(src), Snippet>){
		return src;
	}
	else{
		return src.template copyAs<Snippet>();
	}
});
auto operator""_S(const char* compare, size_t size) {
	return Snippet(compare, size);
}

int main(int argc, char* argv[]) {

	if(argc != 3){
		std::cout << "usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE\n";
		return EXIT_FAILURE;
	}

	auto tStart = tic();

	const auto _ = ometa::capture(*(" "_L | "\t"_L | "\n"_L)) >= toSnippet;

	const auto identStart = ometa::range(('A'), ('Z')) | ometa::range(('a'), ('z')) | "_"_L | "::"_L;
	const auto identContinue = identStart | ometa::range(('0'), ('9'));
	const auto identifier = ometa::capture(identStart > *identContinue) >= toSnippet;
	const auto reference = identifier > ~"'"_L >= ometa::action([](auto value){return "ometa::ptr("_S + value + ")"_S;});

	const auto cppChar = ~"\\"_L > ~ometa::any() | ~ometa::any();
	const auto cppLiteral = ometa::capture(~"'"_L > !"'"_L > ~cppChar > ~"'"_L
		| ~"\""_L > ~*(!"\""_L > ~cppChar) > ~"\""_L) >= toSnippet;

	const auto valueRef = ~"$"_L >= ometa::action([](auto value){return "value"_S;});
	const auto indexedValueRef = ~"$"_L > ometa::capture(+ometa::range(('0'), ('9'))) >= toSnippet >= ometa::action([](auto value){return "ometa::pick<"_S + value + "-1>(value)"_S;});

	auto cpp = ometa::declare<std::string, Snippet>();

	auto ruleForwardDecl = ometa::declare<std::string, Snippet>();
	auto ruleDefinition = ometa::declare<std::string, Snippet>();
	auto ruleRedefinition = ometa::declare<std::string, Snippet>();

	const auto bracedCpp = ~"{"_L > ometa::ptr(cpp) > ~"}"_L;
	
	*cpp = *(ometa::ptr(ruleForwardDecl)
		| ometa::ptr(ruleDefinition)
		| ometa::ptr(ruleRedefinition)
		| identifier
		| cppLiteral
		| bracedCpp >= ometa::action([](auto value){ return "{"_S + value + "}"_S; })
		| indexedValueRef
		| valueRef
		| !"}"_L > ometa::any() >= toSnippet) >= ometa::concat;

	const auto any = "."_L >= ometa::action([](auto value){return "ometa::any()"_S;});
	const auto epsilon = "()"_L >= ometa::action([](auto value){return "ometa::epsilon()"_S;});

	const auto character = ~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\"_L)
		| !"\\"_L > ~ometa::any();
	const auto literal = ometa::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet > (ometa::epsilon() >= ometa::action([](auto value){return "_L"_S;})) >= ometa::concat;

	const auto range = bracedCpp > ~_ > ~".."_L > ~_ > bracedCpp >= ometa::action([](auto value){
		return "ometa::range(("_S + ometa::pick<1-1>(value) + "), ("_S + ometa::pick<2-1>(value) + "))"_S;
	});

	auto choice = ometa::declare<std::string, Snippet>();
	const auto parenthesized = ~"("_L > ~_ > ometa::ptr(choice) > ~_ > ~")"_L >= ometa::action([](auto value){ return "("_S + value + ")"_S; });
	const auto capture = ~"<"_L > ~_ > ometa::ptr(choice) > ~_ > ~">"_L >= ometa::action([](auto value){ return "ometa::capture("_S + value + ")"_S; });

	const auto action = identifier >= toSnippet
		| bracedCpp >= ometa::action([](auto value){ return "ometa::action([](auto value){"_S + value + "})"_S; });
	const auto predicate = ~"?"_L > ~_ > (identifier >= toSnippet
		| bracedCpp >= ometa::action([](auto value){ return "[](auto value){"_S + value + "}"_S; }));

	const auto freeActionOrPredicate = (ometa::epsilon() >= ometa::action([](auto value){ return "(ometa::epsilon()"_S; })) > ((ometa::epsilon() >= ometa::action([](auto value){return " >= "_S;})) > action
		| (ometa::epsilon() >= ometa::action([](auto value){return " <= "_S;})) > predicate) > (ometa::epsilon() >= ometa::action([](auto value){return ")"_S;})) >= ometa::concat;

	const auto boundActionOrPredicate = ~"->"_L > ~_ > ((ometa::epsilon() >= ometa::action([](auto value){return " >= "_S;})) > action
		| (ometa::epsilon() >= ometa::action([](auto value){return " <= "_S;})) > predicate) >= ometa::concat;

	const auto primary = reference
		| identifier
		| any
		| epsilon
		| literal
		| range
		| capture
		| freeActionOrPredicate
		| parenthesized;

	const auto postfix = primary > -("?"_L >= ometa::action([](auto value){return "-"_S;})
		| "*"_L >= ometa::action([](auto value){return "*"_S;})
		| "+"_L >= ometa::action([](auto value){return "+"_S;})) >= ometa::action([](auto value){
		if (ometa::pick<2-1>(value).size() == 0) {
			return ometa::pick<1-1>(value);
		}
		else {
			return ometa::pick<2-1>(value)[0] + ometa::pick<1-1>(value);
		}
	});

	const auto prefix = ometa::capture(-("&"_L | "!"_L | "~"_L)) >= toSnippet > ~_ > postfix >= ometa::concat;

	const auto sequence = prefix > *((~_ > ((ometa::epsilon() >= ometa::action([](auto value){return " > "_S;})) > prefix >= ometa::concat
			| boundActionOrPredicate)) >= ometa::concat) >= ometa::concat;

	*choice = sequence > *(ometa::capture(_ > "|"_L > _) >= toSnippet > sequence >= ometa::concat) >= ometa::concat;

	*ruleForwardDecl = identifier > ~"'"_L > ~_ > ~":"_L > ~_ > bracedCpp > ~_ > ~"->"_L > ~_ > bracedCpp > ~_ > ~";"_L >= ometa::action([](auto value){
			return "auto "_S + ometa::pick<1-1>(value) + " = ometa::declare<"
				+ ometa::pick<2-1>(value) + ", " + ometa::pick<3-1>(value) + ">();";
		});

	*ruleDefinition = identifier > ~_ > ~":="_L > ~_ > ometa::ptr(choice) > ~_ > ~";"_L >= ometa::action([](auto value){ return "const auto "_S + ometa::pick<1-1>(value) + " = "_S + ometa::pick<2-1>(value) + ";"; });

	*ruleRedefinition = identifier > ~"'"_L > ~_ > ~"=>"_L > ~_ > ometa::ptr(choice) > ~_ > ~";"_L >= ometa::action([](auto value){ return "*"_S + ometa::pick<1-1>(value) + " = "_S + ometa::pick<2-1>(value) + ";"; });

	std::cout << "setup: ";
	toc(tStart);


	auto code = ometa::readFile(argv[1]);

	auto result = cpp->parse(code);
	if (result) {
		try {
			auto backup = ometa::readFile(argv[2]);
			std::time_t time = std::time({});
			char timeString[std::size("yyyy_mm_dd__hh_mm_ssZ")];
			std::strftime(std::data(timeString), std::size(timeString),
				"%Y_%m_%d__%H_%M_%S", std::gmtime(&time));
			ometa::writeFile(std::string(argv[2]) + "." + timeString + ".backup", backup);
		}
		catch (...) {}

		ometa::writeFile(argv[2], *result);
	}
	else {
		std::cout << "fail\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
