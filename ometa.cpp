

#include "ometa.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <ctime>

using ViewTree = ometa::ViewTree<std::string_view>;

int main(int argc, char* argv[]) {

	char* inputPath;
	char* outputPath;

	if(argc == 3 || argc == 4){
		inputPath = argv[1];
		outputPath = argv[2];
		if(argc == 4){
			std::string v = argv[3];
			if(v=="-v"){ometa::globalDebugLevel = 1;}
			else if(v=="-vv"){ometa::globalDebugLevel = 2;}
			else if(v=="-vvv"){ometa::globalDebugLevel = 3;}
			else{
				std::cout << "unknown verbosity setting: " << v << "; expected -v, -vv or -vvv\n";
				return EXIT_FAILURE;
			}
		}
	}
	else{
		std::cout << "usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE [-v[v[v]]]\n";
		return EXIT_FAILURE;
	}

	DECL_DEBUG_TAG(RULE_comment, "comment", true); const auto comment = ometa::rule<RULE_comment>("/*"_lit_ > *(!"*/"_lit_ > ometa::any()) > "*/"_lit_| "//"_lit_ > *(!"\n"_lit_ > ometa::any()) > "\n"_lit_);

	DECL_DEBUG_TAG(RULE__, "_", true); const auto _ = ometa::rule<RULE__>(*(" "_lit_| "\t"_lit_| "\n"_lit_| comment));

	DECL_DEBUG_TAG(RULE_identStart, "identStart", true); const auto identStart = ometa::rule<RULE_identStart>(ometa::range(('A'), ('Z'))| ometa::range(('a'), ('z'))| "_"_lit_| "::"_lit_);
	DECL_DEBUG_TAG(RULE_identContinue, "identContinue", true); const auto identContinue = ometa::rule<RULE_identContinue>(identStart| ometa::range(('0'), ('9')));
	DECL_DEBUG_TAG(RULE_identifier, "identifier", true); const auto identifier = ometa::rule<RULE_identifier>(ometa::capture(identStart > *identContinue));

	DECL_DEBUG_TAG(RULE_cppChar, "cppChar", true); const auto cppChar = ometa::rule<RULE_cppChar>("\\"_lit_ > ometa::any()| ometa::any());
	DECL_DEBUG_TAG(RULE_cppLiteral, "cppLiteral", true); const auto cppLiteral = ometa::rule<RULE_cppLiteral>(ometa::capture("'"_lit_ > *(!"'"_lit_ > cppChar) > "'"_lit_| "\""_lit_ > *(!"\""_lit_ > cppChar) > "\""_lit_));
	
	DECL_DEBUG_TAG(RULE_viewTreeLiteral, "viewTreeLiteral", true); const auto viewTreeLiteral = ometa::rule<RULE_viewTreeLiteral>("`"_lit_ > ometa::action([](auto value, auto& context){return "\""_tree_;}) > ometa::capture(*(!"`"_lit_ > cppChar)) > "`"_lit_ > ometa::action([](auto value, auto& context){return "\"_tree_"_tree_;}) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_valueReference, "valueReference", true); const auto valueReference = ometa::rule<RULE_valueReference>("$"_lit_ >= ometa::action([](auto value, auto& context){return "value"_tree_;}));
	DECL_DEBUG_TAG(RULE_indexedValueReference, "indexedValueReference", true); const auto indexedValueReference = ometa::rule<RULE_indexedValueReference>("$"_lit_ > ometa::capture(+ometa::range(('0'), ('9'))) >= ometa::action([](auto value, auto& context){return "ometa::pick<"_tree_ + value + ">(value)"_tree_;}));
	DECL_DEBUG_TAG(RULE_taggedValueReference, "taggedValueReference", true); const auto taggedValueReference = ometa::rule<RULE_taggedValueReference>("$"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + value + "\">(value)"_tree_;}));

	DECL_DEBUG_TAG(RULE_cppExpression, "cppExpression", true); auto cppExpression = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_cppExpressionWithParens, "cppExpressionWithParens", true); const auto cppExpressionWithParens = ometa::rule<RULE_cppExpressionWithParens>("("_lit_ > cppExpression > ")"_lit_ >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;}));
	DECL_DEBUG_TAG(RULE_cppExpressionWithBrackets, "cppExpressionWithBrackets", true); const auto cppExpressionWithBrackets = ometa::rule<RULE_cppExpressionWithBrackets>("["_lit_ > cppExpression > "]"_lit_ >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;}));
	DECL_DEBUG_TAG(RULE_cppExpressionBetweenBraces, "cppExpressionBetweenBraces", true); const auto cppExpressionBetweenBraces = ometa::rule<RULE_cppExpressionBetweenBraces>("{"_lit_ > cppExpression > "}"_lit_);
	DECL_DEBUG_TAG(RULE_cppExpressionWithBraces, "cppExpressionWithBraces", true); const auto cppExpressionWithBraces = ometa::rule<RULE_cppExpressionWithBraces>(cppExpressionBetweenBraces >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;}));
	DECL_DEBUG_TAG(RULE_predicatecppExpression, "predicatecppExpression", true); const auto predicatecppExpression = ometa::rule<RULE_predicatecppExpression>("{"_lit_ > _ > "?"_lit_ > cppExpression > "}"_lit_);

	DECL_DEBUG_TAG(RULE_contextReference, "contextReference", true); const auto contextReference = ometa::rule<RULE_contextReference>("@"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "context."_tree_ + value;}));

	cppExpression.define(ometa::rule<RULE_cppExpression>(*(identifier >= ometa::predicate([](auto value, auto& context){return  value != "return";})
		| contextReference| viewTreeLiteral| cppLiteral| cppExpressionWithParens| cppExpressionWithBrackets| cppExpressionWithBraces| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > !";"_lit_ > ometa::capture(ometa::any())) >= ometa::concat));

	DECL_DEBUG_TAG(RULE_cppCode, "cppCode", true); auto cppCode = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_cppCodeWithParens, "cppCodeWithParens", true); const auto cppCodeWithParens = ometa::rule<RULE_cppCodeWithParens>("("_lit_ > cppCode > ")"_lit_ >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;}));
	DECL_DEBUG_TAG(RULE_cppCodeWithBrackets, "cppCodeWithBrackets", true); const auto cppCodeWithBrackets = ometa::rule<RULE_cppCodeWithBrackets>("["_lit_ > cppCode > "]"_lit_ >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;}));
	DECL_DEBUG_TAG(RULE_cppCodeBetweenBraces, "cppCodeBetweenBraces", true); const auto cppCodeBetweenBraces = ometa::rule<RULE_cppCodeBetweenBraces>("{"_lit_ > cppCode > "}"_lit_);
	DECL_DEBUG_TAG(RULE_cppCodeWithBraces, "cppCodeWithBraces", true); const auto cppCodeWithBraces = ometa::rule<RULE_cppCodeWithBraces>(cppCodeBetweenBraces >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;}));
	DECL_DEBUG_TAG(RULE_predicateCppCode, "predicateCppCode", true); const auto predicateCppCode = ometa::rule<RULE_predicateCppCode>("{"_lit_ > _ > "?"_lit_ > cppCode > "}"_lit_);

	DECL_DEBUG_TAG(RULE_ruleForwardDecl, "ruleForwardDecl", true); auto ruleForwardDecl = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_ruleDefinition, "ruleDefinition", true); auto ruleDefinition = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_ruleAssignment, "ruleAssignment", true); auto ruleAssignment = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_macroDefinition, "macroDefinition", true); auto macroDefinition = ometa::recursive<std::string_view, ViewTree>();

	cppCode.define(ometa::rule<RULE_cppCode>(*(ruleForwardDecl| ruleDefinition| ruleAssignment| macroDefinition| contextReference| identifier| viewTreeLiteral| cppLiteral| cppCodeWithParens| cppCodeWithBrackets| cppCodeWithBraces| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > ometa::capture(ometa::any())) >= ometa::concat));

	DECL_DEBUG_TAG(RULE_any, "any", true); const auto any = ometa::rule<RULE_any>("."_lit_ >= ometa::action([](auto value, auto& context){return "ometa::any()"_tree_;}));
	DECL_DEBUG_TAG(RULE_epsilon, "epsilon", true); const auto epsilon = ometa::rule<RULE_epsilon>("("_lit_ > _ > ")"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::epsilon()"_tree_;}));

	DECL_DEBUG_TAG(RULE_literalCharacter, "literalCharacter", true); const auto literalCharacter = ometa::rule<RULE_literalCharacter>(ometa::capture("\\"_lit_ > ("n"_lit_| "r"_lit_| "t"_lit_| "'"_lit_| "\""_lit_| "\\"_lit_))| "\""_lit_ >= ometa::action([](auto value, auto& context){return "\\\""_tree_;})
		| ometa::capture(!"\\"_lit_ > ometa::any()));

	DECL_DEBUG_TAG(RULE_literal, "literal", true); const auto literal = ometa::rule<RULE_literal>(cppLiteral > ometa::action([](auto value, auto& context){return "_lit_"_tree_;}) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_range, "range", true); const auto range = ometa::rule<RULE_range>(cppCodeBetweenBraces > _ > ".."_lit_ > _ > cppCodeBetweenBraces >= ometa::action([](auto value, auto& context){return 
		"ometa::range(("_tree_ + ometa::pick<0>(value) + "), ("_tree_ + ometa::pick<1>(value) + "))"_tree_
	;}));

	DECL_DEBUG_TAG(RULE_expression, "expression", true); auto expression = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_expressionWithParens, "expressionWithParens", true); const auto expressionWithParens = ometa::rule<RULE_expressionWithParens>("("_lit_ > _ > expression > _ > ")"_lit_ >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;}));
	DECL_DEBUG_TAG(RULE_capture, "capture", true); const auto capture = ometa::rule<RULE_capture>("<"_lit_ > _ > expression > _ > ">"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::capture("_tree_ + value + ")"_tree_;}));

	DECL_DEBUG_TAG(RULE_action, "action", true); const auto action = ometa::rule<RULE_action>(identifier| cppExpressionBetweenBraces >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| cppCodeBetweenBraces >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));
	DECL_DEBUG_TAG(RULE_predicate, "predicate", true); const auto predicate = ometa::rule<RULE_predicate>(identifier| predicatecppExpression >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| predicateCppCode >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));

	DECL_DEBUG_TAG(RULE_parametricPredicateOrAction, "parametricPredicateOrAction", true); const auto parametricPredicateOrAction = ometa::rule<RULE_parametricPredicateOrAction>("=>"_lit_ > ometa::action([](auto value, auto& context){return " >= "_tree_;}) > _ > (predicate| action) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_macroCall, "macroCall", true); const auto macroCall = ometa::rule<RULE_macroCall>(identifier > _ > "["_lit_ > ometa::action([](auto value, auto& context){return "("_tree_;}) > _ > expression > _ > *(ometa::capture(","_lit_) > ometa::action([](auto value, auto& context){return " "_tree_;}) > _ > expression) > _ > "]"_lit_ > ometa::action([](auto value, auto& context){return ")"_tree_;}) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_primary, "primary", true); const auto primary = ometa::rule<RULE_primary>(macroCall| any| epsilon| literal| range| capture| predicate| action| expressionWithParens);

	DECL_DEBUG_TAG(RULE_optional, "optional", true); const auto optional = ometa::rule<RULE_optional>("?"_lit_ >= ometa::action([](auto value, auto& context){return "-"_tree_;}));
	DECL_DEBUG_TAG(RULE_zeroOrMore, "zeroOrMore", true); const auto zeroOrMore = ometa::rule<RULE_zeroOrMore>("*"_lit_ >= ometa::action([](auto value, auto& context){return "*"_tree_;}));
	DECL_DEBUG_TAG(RULE_oneOrMore, "oneOrMore", true); const auto oneOrMore = ometa::rule<RULE_oneOrMore>("+"_lit_ >= ometa::action([](auto value, auto& context){return "+"_tree_;}));
	DECL_DEBUG_TAG(RULE_repetition, "repetition", true); const auto repetition = ometa::rule<RULE_repetition>(optional| zeroOrMore| oneOrMore);
	DECL_DEBUG_TAG(RULE_tag, "tag", true); const auto tag = ometa::rule<RULE_tag>(":"_lit_ > _ > identifier);

	DECL_DEBUG_TAG(RULE_digit, "digit", true); const auto digit = ometa::rule<RULE_digit>(ometa::range(('0'), ('9')));
	DECL_DEBUG_TAG(RULE_debugMark, "debugMark", true); const auto debugMark = ometa::rule<RULE_debugMark>(ometa::capture(digit > -digit > -digit));

	DECL_DEBUG_TAG(RULE_debugMarkedParser, "debugMarkedParser", true); const auto debugMarkedParser = ometa::rule<RULE_debugMarkedParser>(-(debugMark > _) > primary >= ometa::action([](auto value, auto& context){
			if (ometa::pick<0>(value).size() == 0){return ometa::pick<1>(value);}
			else {return "ometa::logger(\""_tree_ + ometa::pick<0>(value)[0] + "\", "_tree_ + ometa::pick<1>(value) + ")"_tree_;}
		}));

	DECL_DEBUG_TAG(RULE_postfixed, "postfixed", true); const auto postfixed = ometa::rule<RULE_postfixed>(debugMarkedParser > _ > -tag > _ > -repetition > _ > -tag >= ometa::action([](auto value, auto& context){ 
			auto result = ometa::pick<0>(value);
			if (ometa::pick<1>(value).size() == 1) {result = "ometa::tagResult<\""_tree_ + ometa::pick<1>(value)[0] + "\">("_tree_ + result + ")"_tree_;}
			if (ometa::pick<2>(value).size() == 1) {result = ometa::pick<2>(value)[0] + result;}
			if (ometa::pick<3>(value).size() == 1) {result = "ometa::tagResult<\""_tree_ + ometa::pick<3>(value)[0] + "\">("_tree_ + result + ")"_tree_;}
			return result;
		}));

	DECL_DEBUG_TAG(RULE_prefixed, "prefixed", true); const auto prefixed = ometa::rule<RULE_prefixed>(ometa::capture(-("&"_lit_| "!"_lit_| "~"_lit_)) > _ > postfixed >= ometa::concat);

	DECL_DEBUG_TAG(RULE_sequence, "sequence", true); const auto sequence = ometa::rule<RULE_sequence>(prefixed > *(_ > (ometa::action([](auto value, auto& context){return " > "_tree_;}) > prefixed >= ometa::concat
			| parametricPredicateOrAction)) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_choice, "choice", true); const auto choice = ometa::rule<RULE_choice>(sequence > *(ometa::capture(_ > "|"_lit_ > _) > sequence) >= ometa::concat);

	expression.define(ometa::rule<RULE_expression>(choice));

	ruleForwardDecl.define(ometa::rule<RULE_ruleForwardDecl>("%"_lit_ > _ > identifier > _ > ":"_lit_ > _ > cppExpressionBetweenBraces > _ > "->"_lit_ > _ > cppExpressionBetweenBraces > _ > ";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"DECL_DEBUG_TAG(RULE_"_tree_ + ometa::pick<0>(value) + ", \""_tree_ + ometa::pick<0>(value) + "\", true); auto "_tree_ + ometa::pick<0>(value) + " = ometa::recursive<"_tree_
				+ ometa::pick<1>(value) + ", "_tree_ + ometa::pick<2>(value) + ">();"_tree_
		;})));

	ruleDefinition.define(ometa::rule<RULE_ruleDefinition>(-(debugMark > _) > identifier > _ > ":="_lit_ > _ > expression > _ > ";"_lit_ >= ometa::action([](auto value, auto& context){
			auto assignee = "DECL_DEBUG_TAG(RULE_"_tree_ + ometa::pick<1>(value) + ", \""_tree_ + ometa::pick<1>(value) + "\", true); const auto "_tree_ + ometa::pick<1>(value);
			auto body = "ometa::rule<RULE_"_tree_ + ometa::pick<1>(value) + ">("_tree_ + ometa::pick<2>(value) + ")"_tree_;
			if (ometa::pick<0>(value).size() == 0){ return assignee + " = "_tree_ + body + ";"_tree_; }
			else { return assignee + " = ometa::logger(\""_tree_ + ometa::pick<0>(value)[0] + "\", "_tree_ + body + ");"_tree_; }
		})));

	ruleAssignment.define(ometa::rule<RULE_ruleAssignment>(-(debugMark > _) > "%"_lit_ > _ > identifier > _ > ":="_lit_ > _ > expression > _ > ";"_lit_ >= ometa::action([](auto value, auto& context){
			auto body = "ometa::rule<RULE_"_tree_ + ometa::pick<1>(value) + ">("_tree_ + ometa::pick<2>(value) + ")"_tree_;
			if (ometa::pick<0>(value).size() == 0){ return ometa::pick<1>(value) + ".define("_tree_ + body + ");"_tree_; }
			else{ return ometa::pick<1>(value) + ".define(ometa::logger(\""_tree_ + ometa::pick<0>(value)[0] + "\", "_tree_ + body + "));"_tree_; }
		})));

	DECL_DEBUG_TAG(RULE_macroParameterList, "macroParameterList", true); const auto macroParameterList = ometa::rule<RULE_macroParameterList>("["_lit_ > _ > ometa::action([](auto value, auto& context){return "auto "_tree_;}) > identifier > _ > *(","_lit_ > _ > identifier >= ometa::action([](auto value, auto& context){return ", auto "_tree_ + value;})) > _ > "]"_lit_ >= ometa::concat);
	macroDefinition.define(ometa::rule<RULE_macroDefinition>(identifier > _ > macroParameterList > _ > ":="_lit_ > _ > expression > _ > ";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"const auto "_tree_ + ometa::pick<0>(value) + " = [=]("_tree_ + ometa::pick<1>(value) + "){return "_tree_ + ometa::pick<2>(value) + ";};"_tree_
		;})));

	auto code = ometa::readFile(inputPath);

	auto result = cppCode.parse(code);
	if (result) {
		ometa::writeFile(outputPath, *result);
	}
	else {
		std::cout << "fail\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
