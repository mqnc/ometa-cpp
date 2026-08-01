

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

	DECL_DEBUG_TAG(RULE_comment, "comment", true); const auto comment = ometa::rule<RULE_comment>(ometa::capture(~"/*"_lit_ > *(!~"*/"_lit_ > ~ometa::any()) > ~"*/"_lit_)| ometa::capture(~"//"_lit_ > *(!~"\n"_lit_ > ~ometa::any()) > ~"\n"_lit_));

	DECL_DEBUG_TAG(RULE_whitespace, "whitespace", true); const auto whitespace = ometa::rule<RULE_whitespace>(ometa::capture(*(" "_lit_| "\t"_lit_| "\n"_lit_| comment)));
	DECL_DEBUG_TAG(RULE__, "_", true); const auto _ = ometa::rule<RULE__>(~whitespace);

	DECL_DEBUG_TAG(RULE_identStart, "identStart", true); const auto identStart = ometa::rule<RULE_identStart>(ometa::range(('A'), ('Z'))| ometa::range(('a'), ('z'))| "_"_lit_| "::"_lit_);
	DECL_DEBUG_TAG(RULE_identContinue, "identContinue", true); const auto identContinue = ometa::rule<RULE_identContinue>(identStart| ometa::range(('0'), ('9')));
	DECL_DEBUG_TAG(RULE_identifier, "identifier", true); const auto identifier = ometa::rule<RULE_identifier>(ometa::capture(identStart > *identContinue));
	DECL_DEBUG_TAG(RULE_reference, "reference", true); const auto reference = ometa::rule<RULE_reference>(identifier > ~"^"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::ptr("_tree_ + value + ")"_tree_;}));

	DECL_DEBUG_TAG(RULE_cppChar, "cppChar", true); const auto cppChar = ometa::rule<RULE_cppChar>(ometa::capture(~"\\"_lit_ > ~ometa::any()| ~ometa::any()));
	DECL_DEBUG_TAG(RULE_cppLiteral, "cppLiteral", true); const auto cppLiteral = ometa::rule<RULE_cppLiteral>(ometa::capture(~"\'"_lit_ > ~*(!"'"_lit_ > ~cppChar) > ~"\'"_lit_| ~"\""_lit_ > ~*(!"\""_lit_ > ~cppChar) > ~"\""_lit_));
	
	DECL_DEBUG_TAG(RULE_viewTreeLiteral, "viewTreeLiteral", true); const auto viewTreeLiteral = ometa::rule<RULE_viewTreeLiteral>(~"`"_lit_ > ometa::action([](auto value, auto& context){return "\""_tree_;}) > *(~"\""_lit_ >= ometa::action([](auto value, auto& context){return "\\\""_tree_;})
		| !~"`"_lit_ > cppChar) > ~"`"_lit_ > ometa::action([](auto value, auto& context){return "\"_tree_"_tree_;}) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_valueReference, "valueReference", true); const auto valueReference = ometa::rule<RULE_valueReference>(~"$"_lit_ >= ometa::action([](auto value, auto& context){return "value"_tree_;}));
	DECL_DEBUG_TAG(RULE_indexedValueReference, "indexedValueReference", true); const auto indexedValueReference = ometa::rule<RULE_indexedValueReference>(~"$"_lit_ > ometa::capture(+ometa::range(('0'), ('9'))) >= ometa::action([](auto value, auto& context){return "ometa::pick<"_tree_ + value + ">(value)"_tree_;}));
	DECL_DEBUG_TAG(RULE_taggedValueReference, "taggedValueReference", true); const auto taggedValueReference = ometa::rule<RULE_taggedValueReference>(~"$"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + value + "\">(value)"_tree_;}));

	DECL_DEBUG_TAG(RULE_cppExpression, "cppExpression", true); auto cppExpression = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_parenthesizedCppExpression, "parenthesizedCppExpression", true); const auto parenthesizedCppExpression = ometa::rule<RULE_parenthesizedCppExpression>(~"("_lit_ > cppExpression > ~")"_lit_);
	DECL_DEBUG_TAG(RULE_bracketedCppExpression, "bracketedCppExpression", true); const auto bracketedCppExpression = ometa::rule<RULE_bracketedCppExpression>(~"["_lit_ > cppExpression > ~"]"_lit_);
	DECL_DEBUG_TAG(RULE_bracedCppExpression, "bracedCppExpression", true); const auto bracedCppExpression = ometa::rule<RULE_bracedCppExpression>(~"{"_lit_ > cppExpression > ~"}"_lit_);
	DECL_DEBUG_TAG(RULE_predicateCppExpression, "predicateCppExpression", true); const auto predicateCppExpression = ometa::rule<RULE_predicateCppExpression>(~"{"_lit_ > _ > ~"?"_lit_ > cppExpression > ~"}"_lit_);

	DECL_DEBUG_TAG(RULE_contextTableDeclaration, "contextTableDeclaration", true); const auto contextTableDeclaration = ometa::rule<RULE_contextTableDeclaration>(identifier > _ > ~":"_lit_ > _ > bracedCppExpression > _ > ~"->"_lit_ > _ > bracedCppExpression >= ometa::action([](auto value, auto& context){return 
			"\tometa::makeTagged<\""_tree_ + ometa::pick<0>(value) + "\">"_tree_
			+ "(ometa::ContextTable<"_tree_ + ometa::pick<1>(value) + ", "_tree_ + ometa::pick<2>(value) + ">{})"_tree_
		;}));
	DECL_DEBUG_TAG(RULE_contextValueDeclaration, "contextValueDeclaration", true); const auto contextValueDeclaration = ometa::rule<RULE_contextValueDeclaration>(identifier > _ > ~":"_lit_ > _ > bracedCppExpression > -(_ > ~"="_lit_ > _ > bracedCppExpression) >= ometa::action([](auto value, auto& context){return 
			"\tometa::makeTagged<\""_tree_ + ometa::pick<0>(value) + "\">"_tree_
			+ "(ometa::ContextValue<"_tree_ + ometa::pick<1>(value) + ">{"_tree_
			+ (ometa::pick<2>(value).size() > 0 ? ometa::pick<2>(value)[0] : ""_tree_) + "})"_tree_
		;}));
	DECL_DEBUG_TAG(RULE_contextItemDeclaration, "contextItemDeclaration", true); const auto contextItemDeclaration = ometa::rule<RULE_contextItemDeclaration>(contextTableDeclaration| contextValueDeclaration);
	DECL_DEBUG_TAG(RULE_contextItemDeclarationList, "contextItemDeclarationList", true); const auto contextItemDeclarationList = ometa::rule<RULE_contextItemDeclarationList>(contextItemDeclaration > _ > *(","_lit_ > ometa::action([](auto value, auto& context){return "\n"_tree_;}) > _ > contextItemDeclaration) >= ometa::concat);
	DECL_DEBUG_TAG(RULE_contextDeclaration, "contextDeclaration", true); const auto contextDeclaration = ometa::rule<RULE_contextDeclaration>(identifier > ~"@"_lit_ > _ > ~":"_lit_ > _ > contextItemDeclarationList > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return "auto "_tree_ + ometa::pick<0>(value) + " = ometa::Context(\n"_tree_ + ometa::pick<1>(value) + "\n);"_tree_;}));

	DECL_DEBUG_TAG(RULE_contextReference, "contextReference", true); const auto contextReference = ometa::rule<RULE_contextReference>(~"@"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + value + "\">(context)"_tree_;}));
	DECL_DEBUG_TAG(RULE_outsideContextReference, "outsideContextReference", true); const auto outsideContextReference = ometa::rule<RULE_outsideContextReference>(identifier > ~"@"_lit_ > identifier >= ometa::action([](auto value, auto& context){return "ometa::pick<\""_tree_ + ometa::pick<1>(value) + "\">("_tree_ + ometa::pick<0>(value) + ")"_tree_;}));

	cppExpression.define(ometa::rule<RULE_cppExpression>(*(identifier > ometa::predicate([](auto value, auto& context){return  value && *value != "return";})| contextReference| viewTreeLiteral| cppLiteral| parenthesizedCppExpression >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppExpression >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;})
		| bracedCppExpression >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;})
		| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > !";"_lit_ > ometa::any()) >= ometa::concat));

	DECL_DEBUG_TAG(RULE_cppCode, "cppCode", true); auto cppCode = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_parenthesizedCppCode, "parenthesizedCppCode", true); const auto parenthesizedCppCode = ometa::rule<RULE_parenthesizedCppCode>(~"("_lit_ > cppCode > ~")"_lit_);
	DECL_DEBUG_TAG(RULE_bracketedCppCode, "bracketedCppCode", true); const auto bracketedCppCode = ometa::rule<RULE_bracketedCppCode>(~"["_lit_ > cppCode > ~"]"_lit_);
	DECL_DEBUG_TAG(RULE_bracedCppCode, "bracedCppCode", true); const auto bracedCppCode = ometa::rule<RULE_bracedCppCode>(~"{"_lit_ > cppCode > ~"}"_lit_);
	DECL_DEBUG_TAG(RULE_predicateCppCode, "predicateCppCode", true); const auto predicateCppCode = ometa::rule<RULE_predicateCppCode>(~"{"_lit_ > _ > ~"?"_lit_ > cppCode > ~"}"_lit_);

	DECL_DEBUG_TAG(RULE_ruleForwardDecl, "ruleForwardDecl", true); auto ruleForwardDecl = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_ruleDefinition, "ruleDefinition", true); auto ruleDefinition = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_ruleRedefinition, "ruleRedefinition", true); auto ruleRedefinition = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_macroDefinition, "macroDefinition", true); auto macroDefinition = ometa::recursive<std::string_view, ViewTree>();

	cppCode.define(ometa::rule<RULE_cppCode>(*(ruleForwardDecl| ruleDefinition| ruleRedefinition| macroDefinition| contextDeclaration| contextReference| outsideContextReference| identifier| viewTreeLiteral| cppLiteral| parenthesizedCppCode >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;})
		| bracketedCppCode >= ometa::action([](auto value, auto& context){return "["_tree_ + value + "]"_tree_;})
		| bracedCppCode >= ometa::action([](auto value, auto& context){return "{"_tree_ + value + "}"_tree_;})
		| indexedValueReference| taggedValueReference| valueReference| !")"_lit_ > !"]"_lit_ > !"}"_lit_ > ometa::any()) >= ometa::concat));

	DECL_DEBUG_TAG(RULE_any, "any", true); const auto any = ometa::rule<RULE_any>("."_lit_ >= ometa::action([](auto value, auto& context){return "ometa::any()"_tree_;}));
	DECL_DEBUG_TAG(RULE_epsilon, "epsilon", true); const auto epsilon = ometa::rule<RULE_epsilon>("()"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::epsilon()"_tree_;}));

	DECL_DEBUG_TAG(RULE_literalCharacter, "literalCharacter", true); const auto literalCharacter = ometa::rule<RULE_literalCharacter>(ometa::capture("\\"_lit_ > ("n"_lit_| "r"_lit_| "t"_lit_| "'"_lit_| "\""_lit_| "\\"_lit_))| "\""_lit_ >= ometa::action([](auto value, auto& context){return "\\\""_tree_;})
		| ometa::capture(!"\\"_lit_ > ometa::any()));

	DECL_DEBUG_TAG(RULE_ignoredLiteral, "ignoredLiteral", true); const auto ignoredLiteral = ometa::rule<RULE_ignoredLiteral>(~"\'"_lit_ > ometa::action([](auto value, auto& context){return "~\""_tree_;}) > *(!"'"_lit_ > literalCharacter) > ~"\'"_lit_ > ometa::action([](auto value, auto& context){return "\"_lit_"_tree_;}) >= ometa::concat);
	DECL_DEBUG_TAG(RULE_capturedLiteral, "capturedLiteral", true); const auto capturedLiteral = ometa::rule<RULE_capturedLiteral>(ometa::capture("\""_lit_ > *(!"\""_lit_ > literalCharacter) > "\""_lit_) > ometa::action([](auto value, auto& context){return "_lit_"_tree_;}) >= ometa::concat);
	DECL_DEBUG_TAG(RULE_literal, "literal", true); const auto literal = ometa::rule<RULE_literal>(ignoredLiteral| capturedLiteral);

	DECL_DEBUG_TAG(RULE_range, "range", true); const auto range = ometa::rule<RULE_range>(bracedCppCode > _ > ~".."_lit_ > _ > bracedCppCode >= ometa::action([](auto value, auto& context){return 
		"ometa::range(("_tree_ + ometa::pick<0>(value) + "), ("_tree_ + ometa::pick<1>(value) + "))"_tree_
	;}));

	DECL_DEBUG_TAG(RULE_expression, "expression", true); auto expression = ometa::recursive<std::string_view, ViewTree>();
	DECL_DEBUG_TAG(RULE_parenthesized, "parenthesized", true); const auto parenthesized = ometa::rule<RULE_parenthesized>(~"("_lit_ > _ > expression > _ > ~")"_lit_ >= ometa::action([](auto value, auto& context){return "("_tree_ + value + ")"_tree_;}));
	DECL_DEBUG_TAG(RULE_capture, "capture", true); const auto capture = ometa::rule<RULE_capture>(~"<"_lit_ > _ > expression > _ > ~">"_lit_ >= ometa::action([](auto value, auto& context){return "ometa::capture("_tree_ + value + ")"_tree_;}));

	DECL_DEBUG_TAG(RULE_action, "action", true); const auto action = ometa::rule<RULE_action>(identifier| bracedCppExpression >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| bracedCppCode >= ometa::action([](auto value, auto& context){return "ometa::action([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));
	DECL_DEBUG_TAG(RULE_predicate, "predicate", true); const auto predicate = ometa::rule<RULE_predicate>(identifier| predicateCppExpression >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){return "_tree_ + value + ";})"_tree_;})
		| predicateCppCode >= ometa::action([](auto value, auto& context){return "ometa::predicate([](auto value, auto& context){"_tree_ + value + "})"_tree_;}));

	DECL_DEBUG_TAG(RULE_parameterizedAction, "parameterizedAction", true); const auto parameterizedAction = ometa::rule<RULE_parameterizedAction>(~"->"_lit_ > ometa::action([](auto value, auto& context){return " >= "_tree_;}) > _ > action >= ometa::concat);

	DECL_DEBUG_TAG(RULE_macroCall, "macroCall", true); const auto macroCall = ometa::rule<RULE_macroCall>(identifier > _ > ~"["_lit_ > ometa::action([](auto value, auto& context){return "("_tree_;}) > _ > expression > _ > *(","_lit_ > ometa::action([](auto value, auto& context){return " "_tree_;}) > _ > expression) > _ > ~"]"_lit_ > ometa::action([](auto value, auto& context){return ")"_tree_;}) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_primary, "primary", true); const auto primary = ometa::rule<RULE_primary>(reference| macroCall| any| epsilon| literal| range| capture| predicate| action| parenthesized);

	DECL_DEBUG_TAG(RULE_optional, "optional", true); const auto optional = ometa::rule<RULE_optional>("?"_lit_ >= ometa::action([](auto value, auto& context){return "-"_tree_;}));
	DECL_DEBUG_TAG(RULE_zeroOrMore, "zeroOrMore", true); const auto zeroOrMore = ometa::rule<RULE_zeroOrMore>("*"_lit_ >= ometa::action([](auto value, auto& context){return "*"_tree_;}));
	DECL_DEBUG_TAG(RULE_oneOrMore, "oneOrMore", true); const auto oneOrMore = ometa::rule<RULE_oneOrMore>("+"_lit_ >= ometa::action([](auto value, auto& context){return "+"_tree_;}));
	DECL_DEBUG_TAG(RULE_repetition, "repetition", true); const auto repetition = ometa::rule<RULE_repetition>(optional| zeroOrMore| oneOrMore);
	DECL_DEBUG_TAG(RULE_tag, "tag", true); const auto tag = ometa::rule<RULE_tag>(~":"_lit_ > _ > identifier);

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
			| parameterizedAction)) >= ometa::concat);

	DECL_DEBUG_TAG(RULE_choice, "choice", true); const auto choice = ometa::rule<RULE_choice>(sequence > *(ometa::capture(whitespace > "|"_lit_ > whitespace) > sequence) >= ometa::concat);

	expression.define(ometa::rule<RULE_expression>(choice));

	ruleForwardDecl.define(ometa::rule<RULE_ruleForwardDecl>(identifier > _ > ~":"_lit_ > _ > bracedCppExpression > _ > ~"=>"_lit_ > _ > bracedCppExpression > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"DECL_DEBUG_TAG(RULE_"_tree_ + ometa::pick<0>(value) + ", \""_tree_ + ometa::pick<0>(value) + "\", true); auto "_tree_ + ometa::pick<0>(value) + " = ometa::recursive<"_tree_
				+ ometa::pick<1>(value) + ", "_tree_ + ometa::pick<2>(value) + ">();"_tree_
		;})));

	ruleDefinition.define(ometa::rule<RULE_ruleDefinition>(-(debugMark > _) > identifier > _ > ~":="_lit_ > _ > expression > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){
			auto assignee = "DECL_DEBUG_TAG(RULE_"_tree_ + ometa::pick<1>(value) + ", \""_tree_ + ometa::pick<1>(value) + "\", true); const auto "_tree_ + ometa::pick<1>(value);
			auto body = "ometa::rule<RULE_"_tree_ + ometa::pick<1>(value) + ">("_tree_ + ometa::pick<2>(value) + ")"_tree_;
			if (ometa::pick<0>(value).size() == 0){ return assignee + " = "_tree_ + body + ";"_tree_; }
			else { return assignee + " = ometa::logger(\""_tree_ + ometa::pick<0>(value)[0] + "\", "_tree_ + body + ");"_tree_; }
		})));

	ruleRedefinition.define(ometa::rule<RULE_ruleRedefinition>(identifier > _ > ~"=>"_lit_ > _ > expression > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return ometa::pick<0>(value) + ".define(ometa::rule<RULE_"_tree_ + ometa::pick<0>(value) + ">("_tree_ + ometa::pick<1>(value) + "));"_tree_;})));

	DECL_DEBUG_TAG(RULE_macroParameterList, "macroParameterList", true); const auto macroParameterList = ometa::rule<RULE_macroParameterList>(~"["_lit_ > _ > ometa::action([](auto value, auto& context){return "auto "_tree_;}) > identifier > _ > *(~","_lit_ > _ > identifier >= ometa::action([](auto value, auto& context){return ", auto "_tree_ + value;})) > _ > ~"]"_lit_ >= ometa::concat);
	macroDefinition.define(ometa::rule<RULE_macroDefinition>(identifier > _ > macroParameterList > _ > ~":="_lit_ > _ > expression > _ > ~";"_lit_ >= ometa::action([](auto value, auto& context){return 
			"const auto "_tree_ + ometa::pick<0>(value) + " = [=]("_tree_ + ometa::pick<1>(value) + "){return "_tree_ + ometa::pick<2>(value) + ";};"_tree_
		;})));

	auto code = ometa::readFile(inputPath);

	auto result = cppCode.parse(code);
	if (result) {
		// make a backup copy if transpiled file already exists

		// try {
		// 	auto backup = ometa::readFile(outputPath);
		// 	std::time_t time = std::time({});
		// 	char timeString[std::size("yyyy_mm_dd__hh_mm_ssZ")];
		// 	std::strftime(std::data(timeString), std::size(timeString),
		// 		"%Y_%m_%d__%H_%M_%S", std::gmtime(&time));
		// 	ometa::writeFile(std::string(outputPath) + "." + timeString + ".backup", backup);
		// }
		// catch (...) {}

		ometa::writeFile(outputPath, *result);
	}
	else {
		std::cout << "fail\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
