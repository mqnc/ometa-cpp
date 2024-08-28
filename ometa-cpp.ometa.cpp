

#include "src/ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

#undef OMETA_LOG
#define OMETA_LOG(p)

using ViewTree = ometa::ViewTree<std::string_view>;

int main(int argc, char* argv[]) {

	if(argc != 3){
		std::cout << "usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE\n";
		return EXIT_FAILURE;
	}

	const auto _ = ometa::capture(*(" "_lit_ | "\t"_lit_ | "\n"_lit_)); OMETA_LOG(_);

	const auto identStart = ometa::range(('A'), ('Z')) | ometa::range(('a'), ('z')) | "_"_lit_ | "::"_lit_; OMETA_LOG(identStart);
	const auto identContinue = identStart | ometa::range(('0'), ('9')); OMETA_LOG(identContinue);
	const auto identifier = ometa::capture(identStart > *identContinue); OMETA_LOG(identifier);
	const auto reference = identifier > ~"^"_lit_ >= ometa::action([](auto value){return "ometa::ptr("_tree_ + value + ")"_tree_;}); OMETA_LOG(reference);

	const auto cppChar = ~"\\"_lit_ > ~ometa::any() | ~ometa::any(); OMETA_LOG(cppChar);
	const auto cppLiteral = ometa::capture(~"'"_lit_ > ~*(!"'"_lit_ > ~cppChar) > ~"'"_lit_
		| ~"\""_lit_ > ~*(!"\""_lit_ > ~cppChar) > ~"\""_lit_); OMETA_LOG(cppLiteral);
	
	const auto viewTreeLiteral = ~"'"_lit_ > ometa::capture("\""_lit_ > ~*(!"'"_lit_ > !"\""_lit_ > ~cppChar) > ~"\""_lit_) > ~"'"_lit_ > ometa::action([](auto value){return "_tree_"_tree_;}) >= ometa::concat; OMETA_LOG(viewTreeLiteral);

	const auto valueRef = ~"$"_lit_ >= ometa::action([](auto value){return "value"_tree_;}); OMETA_LOG(valueRef);
	const auto indexedValueRef = ~"$"_lit_ > ometa::capture(+ometa::range(('0'), ('9'))) >= ometa::action([](auto value){return "ometa::pick<"_tree_ + value + "-1>(value)"_tree_;}); OMETA_LOG(indexedValueRef);

	auto cppExpression = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesizedCppExpression = ~"("_lit_ > ometa::ptr(cppExpression) > ~")"_lit_; OMETA_LOG(parenthesizedCppExpression);
	const auto bracketedCppExpression = ~"["_lit_ > ometa::ptr(cppExpression) > ~"]"_lit_; OMETA_LOG(bracketedCppExpression);
	const auto bracedCppExpression = ~"{"_lit_ > ometa::ptr(cppExpression) > ~"}"_lit_; OMETA_LOG(bracedCppExpression);
	const auto predicateCppExpression = ~"{"_lit_ > ~_ > ~"?"_lit_ > ometa::ptr(cppExpression) > ~"}"_lit_; OMETA_LOG(predicateCppExpression);

	*cppExpression = *(identifier > ometa::predicate([](auto value){return  value && *value != "return";})
		| viewTreeLiteral
		| cppLiteral
		| parenthesizedCppExpression >= ometa::action([](auto value){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppExpression >= ometa::action([](auto value){return "["_tree_ + value + "]"_tree_;})
		| indexedValueRef
		| valueRef
		| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > !";"_lit_ > ometa::any()) >= ometa::concat; OMETA_LOG(*cppExpression);

	auto cppCode = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesizedCppCode = ~"("_lit_ > ometa::ptr(cppCode) > ~")"_lit_; OMETA_LOG(parenthesizedCppCode);
	const auto bracketedCppCode = ~"["_lit_ > ometa::ptr(cppCode) > ~"]"_lit_; OMETA_LOG(bracketedCppCode);
	const auto bracedCppCode = ~"{"_lit_ > ometa::ptr(cppCode) > ~"}"_lit_; OMETA_LOG(bracedCppCode);
	const auto predicateCppCode = ~"{"_lit_ > ~_ > ~"?"_lit_ > ometa::ptr(cppCode) > ~"}"_lit_; OMETA_LOG(predicateCppCode);

	auto ruleForwardDecl = ometa::declare<std::string_view, ViewTree>();
	auto ruleDefinition = ometa::declare<std::string_view, ViewTree>();
	auto ruleRedefinition = ometa::declare<std::string_view, ViewTree>();

	*cppCode = *(ometa::ptr(ruleForwardDecl)
		| ometa::ptr(ruleDefinition)
		| ometa::ptr(ruleRedefinition)
		| identifier
		| cppLiteral
		| parenthesizedCppCode >= ometa::action([](auto value){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppCode >= ometa::action([](auto value){return "["_tree_ + value + "]"_tree_;})
		| bracedCppCode >= ometa::action([](auto value){return "{"_tree_ + value + "}"_tree_;})
		| indexedValueRef
		| valueRef
		| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > ometa::any()) >= ometa::concat; OMETA_LOG(*cppCode);

	const auto any = "."_lit_ >= ometa::action([](auto value){return "ometa::any()"_tree_;}); OMETA_LOG(any);
	const auto epsilon = "()"_lit_ >= ometa::action([](auto value){return "ometa::epsilon()"_tree_;}); OMETA_LOG(epsilon);

	const auto character = ~"\\"_lit_ > ~("n"_lit_ | "r"_lit_ | "t"_lit_ | "\""_lit_ | "\\"_lit_)
		| !"\\"_lit_ > ~ometa::any(); OMETA_LOG(character);
	const auto literal = ometa::capture("\""_lit_ > *(!"\""_lit_ > character) > "\""_lit_) > ometa::action([](auto value){return "_lit_"_tree_;}) >= ometa::concat; OMETA_LOG(literal);

	const auto range = bracedCppCode > ~_ > ~".."_lit_ > ~_ > bracedCppCode >= ometa::action([](auto value){return 
		"ometa::range(("_tree_ + ometa::pick<1-1>(value) + "), ("_tree_ + ometa::pick<2-1>(value) + "))"_tree_
	;}); OMETA_LOG(range);

	auto choice = ometa::declare<std::string_view, ViewTree>();
	const auto parenthesized = ~"("_lit_ > ~_ > ometa::ptr(choice) > ~_ > ~")"_lit_ >= ometa::action([](auto value){return "("_tree_ + value + ")"_tree_;}); OMETA_LOG(parenthesized);
	const auto capture = ~"<"_lit_ > ~_ > ometa::ptr(choice) > ~_ > ~">"_lit_ >= ometa::action([](auto value){return "ometa::capture("_tree_ + value + ")"_tree_;}); OMETA_LOG(capture);

	const auto action = identifier
		| bracedCppExpression >= ometa::action([](auto value){return "ometa::action([](auto value){return "_tree_ + value + ";})"_tree_;})
		| bracedCppCode >= ometa::action([](auto value){return "ometa::action([](auto value){"_tree_ + value + "})"_tree_;}); OMETA_LOG(action);
	const auto predicate = identifier
		| predicateCppExpression >= ometa::action([](auto value){return "ometa::predicate([](auto value){return "_tree_ + value + ";})"_tree_;})
		| predicateCppCode >= ometa::action([](auto value){return "ometa::predicate([](auto value){"_tree_ + value + "})"_tree_;}); OMETA_LOG(predicate);

	const auto parameterizedAction = ~"->"_lit_ > ometa::action([](auto value){return " >= "_tree_;}) > ~_ > action >= ometa::concat; OMETA_LOG(parameterizedAction);

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

	const auto postfix = primary > -("?"_lit_ >= ometa::action([](auto value){return "-"_tree_;})
		| "*"_lit_ >= ometa::action([](auto value){return "*"_tree_;})
		| "+"_lit_ >= ometa::action([](auto value){return "+"_tree_;})) >= ometa::action([](auto value){return  ometa::pick<2-1>(value).size() == 0 ? ometa::pick<1-1>(value) : ometa::pick<2-1>(value)[0] + ometa::pick<1-1>(value) ;}); OMETA_LOG(postfix);

	const auto prefix = ometa::capture(-("&"_lit_ | "!"_lit_ | "~"_lit_)) > ~_ > postfix >= ometa::concat; OMETA_LOG(prefix);

	const auto sequence = prefix > *((~_ > (ometa::action([](auto value){return " > "_tree_;}) > prefix >= ometa::concat
			| parameterizedAction)) >= ometa::concat) >= ometa::concat; OMETA_LOG(sequence);

	*choice = sequence > *(ometa::capture(_ > "|"_lit_ > _) > sequence >= ometa::concat) >= ometa::concat; OMETA_LOG(*choice);

	*ruleForwardDecl = identifier > ~"^"_lit_ > ~_ > ~":"_lit_ > ~_ > bracedCppExpression > ~_ > ~"->"_lit_ > ~_ > bracedCppExpression > ~_ > ~";"_lit_ >= ometa::action([](auto value){return 
			"auto "_tree_ + ometa::pick<1-1>(value) + " = ometa::declare<"_tree_
				+ ometa::pick<2-1>(value) + ", "_tree_ + ometa::pick<3-1>(value) + ">();"_tree_
		;}); OMETA_LOG(*ruleForwardDecl);

	*ruleDefinition = identifier > ~_ > ~":="_lit_ > ~_ > ometa::ptr(choice) > ~_ > ~";"_lit_ >= ometa::action([](auto value){return "const auto "_tree_ + ometa::pick<1-1>(value) + " = "_tree_ + ometa::pick<2-1>(value) + "; OMETA_LOG("_tree_ + ometa::pick<1-1>(value) + ");"_tree_;}); OMETA_LOG(*ruleDefinition);

	*ruleRedefinition = identifier > ~"^"_lit_ > ~_ > ~"=>"_lit_ > ~_ > ometa::ptr(choice) > ~_ > ~";"_lit_ >= ometa::action([](auto value){return "*"_tree_ + ometa::pick<1-1>(value) + " = "_tree_ + ometa::pick<2-1>(value) + "; OMETA_LOG(*"_tree_ + ometa::pick<1-1>(value) + ");"_tree_;}); OMETA_LOG(*ruleRedefinition);

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
