

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

#undef OMETA_LOG
#define OMETA_LOG(p)

using ometa::operator""_L;

using Snippet = std::string;
auto toSnippet = ometa::action([](auto src) {
	return src.template copyAs<Snippet>();
});
auto operator""_S(const char* compare, size_t size) {
	return Snippet(compare, size);
}

int main(int argc, char* argv[]) {

	if(argc != 3){
		std::cout << "usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE\n";
		return EXIT_FAILURE;
	}

	const auto _ = ometa::capture(*(" "_L | "\t"_L | "\n"_L)) >= toSnippet; OMETA_LOG(_);

	const auto identStart = ometa::range(('A'), ('Z')) | ometa::range(('a'), ('z')) | "_"_L | "::"_L; OMETA_LOG(identStart);
	const auto identContinue = identStart | ometa::range(('0'), ('9')); OMETA_LOG(identContinue);
	const auto identifier = ometa::capture(identStart > *identContinue) >= toSnippet; OMETA_LOG(identifier);
	const auto reference = identifier > ~"^"_L >= ometa::action([](auto value){return "ometa::ptr("_S + value + ")"_S;}); OMETA_LOG(reference);

	const auto cppChar = ~"\\"_L > ~ometa::any() | ~ometa::any(); OMETA_LOG(cppChar);
	const auto cppLiteral = ometa::capture(~"'"_L > !"'"_L > ~cppChar > ~"'"_L
		| ~"\""_L > ~*(!"\""_L > ~cppChar) > ~"\""_L) >= toSnippet; OMETA_LOG(cppLiteral);

	const auto valueRef = ~"$"_L >= ometa::action([](auto value){return "value"_S;}); OMETA_LOG(valueRef);
	const auto indexedValueRef = ~"$"_L > ometa::capture(+ometa::range(('0'), ('9'))) >= toSnippet >= ometa::action([](auto value){return "ometa::pick<"_S + value + "-1>(value)"_S;}); OMETA_LOG(indexedValueRef);

	auto cppExpression = ometa::declare<std::string, Snippet>();
	const auto parenthesizedCppExpression = ~"("_L > ometa::ptr(cppExpression) > ~")"_L; OMETA_LOG(parenthesizedCppExpression);
	const auto bracketedCppExpression = ~"["_L > ometa::ptr(cppExpression) > ~"]"_L; OMETA_LOG(bracketedCppExpression);
	const auto bracedCppExpression = ~"{"_L > ometa::ptr(cppExpression) > ~"}"_L; OMETA_LOG(bracedCppExpression);
	const auto predicateCppExpression = ~"{"_L > ~_ > ~"?"_L > ometa::ptr(cppExpression) > ~"}"_L; OMETA_LOG(predicateCppExpression);

	*cppExpression = *(identifier > ometa::predicate([](auto value){return  value != "return";})
		| cppLiteral
		| parenthesizedCppExpression >= ometa::action([](auto value){return "("_S + value + ")"_S;})
		| bracketedCppExpression >= ometa::action([](auto value){return "["_S + value + "]"_S;})
		| indexedValueRef
		| valueRef
		| !")"_L > !"]"_L > !"}"_L > !";"_L > ometa::any() >= toSnippet) >= ometa::concat; OMETA_LOG(*cppExpression);

	auto cppCode = ometa::declare<std::string, Snippet>();
	const auto parenthesizedCppCode = ~"("_L > ometa::ptr(cppCode) > ~")"_L; OMETA_LOG(parenthesizedCppCode);
	const auto bracketedCppCode = ~"["_L > ometa::ptr(cppCode) > ~"]"_L; OMETA_LOG(bracketedCppCode);
	const auto bracedCppCode = ~"{"_L > ometa::ptr(cppCode) > ~"}"_L; OMETA_LOG(bracedCppCode);
	const auto predicateCppCode = ~"{"_L > ~_ > ~"?"_L > ometa::ptr(cppCode) > ~"}"_L; OMETA_LOG(predicateCppCode);

	auto ruleForwardDecl = ometa::declare<std::string, Snippet>();
	auto ruleDefinition = ometa::declare<std::string, Snippet>();
	auto ruleRedefinition = ometa::declare<std::string, Snippet>();

	*cppCode = *(ometa::ptr(ruleForwardDecl)
		| ometa::ptr(ruleDefinition)
		| ometa::ptr(ruleRedefinition)
		| identifier
		| cppLiteral
		| parenthesizedCppCode >= ometa::action([](auto value){return "("_S + value + ")"_S;})
		| bracketedCppCode >= ometa::action([](auto value){return "["_S + value + "]"_S;})
		| bracedCppCode >= ometa::action([](auto value){return "{"_S + value + "}"_S;})
		| indexedValueRef
		| valueRef
		| !")"_L > !"]"_L > !"}"_L > ometa::any() >= toSnippet) >= ometa::concat; OMETA_LOG(*cppCode);

	const auto any = "."_L >= ometa::action([](auto value){return "ometa::any()"_S;}); OMETA_LOG(any);
	const auto epsilon = "()"_L >= ometa::action([](auto value){return "ometa::epsilon()"_S;}); OMETA_LOG(epsilon);

	const auto character = ~"\\"_L > ~("n"_L | "r"_L | "t"_L | "\""_L | "\\"_L)
		| !"\\"_L > ~ometa::any(); OMETA_LOG(character);
	const auto literal = ometa::capture("\""_L > *(!"\""_L > character) > "\""_L) >= toSnippet > ometa::action([](auto value){return "_L"_S;}) >= ometa::concat; OMETA_LOG(literal);

	const auto range = bracedCppCode > ~_ > ~".."_L > ~_ > bracedCppCode >= ometa::action([](auto value){return 
		"ometa::range(("_S + ometa::pick<1-1>(value) + "), ("_S + ometa::pick<2-1>(value) + "))"_S
	;}); OMETA_LOG(range);

	auto choice = ometa::declare<std::string, Snippet>();
	const auto parenthesized = ~"("_L > ~_ > ometa::ptr(choice) > ~_ > ~")"_L >= ometa::action([](auto value){return "("_S + value + ")"_S;}); OMETA_LOG(parenthesized);
	const auto capture = ~"<"_L > ~_ > ometa::ptr(choice) > ~_ > ~">"_L >= ometa::action([](auto value){return "ometa::capture("_S + value + ")"_S;}); OMETA_LOG(capture);

	const auto action = identifier
		| bracedCppExpression >= ometa::action([](auto value){return "ometa::action([](auto value){return "_S + value + ";})"_S;})
		| bracedCppCode >= ometa::action([](auto value){return "ometa::action([](auto value){"_S + value + "})"_S;}); OMETA_LOG(action);
	const auto predicate = identifier
		| predicateCppExpression >= ometa::action([](auto value){return "ometa::predicate([](auto value){return "_S + value + ";})"_S;})
		| predicateCppCode >= ometa::action([](auto value){return "ometa::predicate([](auto value){"_S + value + "})"_S;}); OMETA_LOG(predicate);

	const auto parameterizedAction = ~"->"_L > ometa::action([](auto value){return " >= "_S;}) > ~_ > action >= ometa::concat; OMETA_LOG(parameterizedAction);

	const auto primary = reference
		| identifier
		| any
		| epsilon
		| literal
		| range
		| capture
		| predicate
		| action
		| parenthesized; OMETA_LOG(primary);

	const auto postfix = primary > -("?"_L >= ometa::action([](auto value){return "-"_S;})
		| "*"_L >= ometa::action([](auto value){return "*"_S;})
		| "+"_L >= ometa::action([](auto value){return "+"_S;})) >= ometa::action([](auto value){return  ometa::pick<2-1>(value).size() == 0 ? ometa::pick<1-1>(value) : ometa::pick<2-1>(value)[0] + ometa::pick<1-1>(value) ;}); OMETA_LOG(postfix);

	const auto prefix = ometa::capture(-("&"_L | "!"_L | "~"_L)) >= toSnippet > ~_ > postfix >= ometa::concat; OMETA_LOG(prefix);

	const auto sequence = prefix > *((~_ > (ometa::action([](auto value){return " > "_S;}) > prefix >= ometa::concat
			| parameterizedAction)) >= ometa::concat) >= ometa::concat; OMETA_LOG(sequence);

	*choice = sequence > *(ometa::capture(_ > "|"_L > _) >= toSnippet > sequence >= ometa::concat) >= ometa::concat; OMETA_LOG(*choice);

	*ruleForwardDecl = identifier > ~"^"_L > ~_ > ~":"_L > ~_ > bracedCppExpression > ~_ > ~"->"_L > ~_ > bracedCppExpression > ~_ > ~";"_L >= ometa::action([](auto value){return 
			"auto "_S + ometa::pick<1-1>(value) + " = ometa::declare<"
				+ ometa::pick<2-1>(value) + ", " + ometa::pick<3-1>(value) + ">();"
		;}); OMETA_LOG(*ruleForwardDecl);

	*ruleDefinition = identifier > ~_ > ~":="_L > ~_ > ometa::ptr(choice) > ~_ > ~";"_L >= ometa::action([](auto value){return "const auto "_S + ometa::pick<1-1>(value) + " = "_S + ometa::pick<2-1>(value) + "; OMETA_LOG("_S + ometa::pick<1-1>(value) + ");"_S;}); OMETA_LOG(*ruleDefinition);

	*ruleRedefinition = identifier > ~"^"_L > ~_ > ~"=>"_L > ~_ > ometa::ptr(choice) > ~_ > ~";"_L >= ometa::action([](auto value){return "*"_S + ometa::pick<1-1>(value) + " = "_S + ometa::pick<2-1>(value) + "; OMETA_LOG(*"_S + ometa::pick<1-1>(value) + ");"_S;}); OMETA_LOG(*ruleRedefinition);

	auto code = ometa::readFile(argv[1]);

	auto result = cppCode->parse(code);
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
