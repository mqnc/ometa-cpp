set -e
echo "creating update folder and copying parser sources..."
mkdir update
cp -r --update=none include update/include
cp --update=none examples/ometa.ometa update/bridge_parser.ometa
cp --update=none examples/ometa.ometa update/new_parser.ometa
cp --update=none examples/test.ometa update/test.ometa
cp --update=none examples/calculator.ometa update/calculator.ometa
if [ ! -f "update/playground.ometa" ]; then
  cat << 'EOF' > update/playground.ometa
#include "ometa.h"

using ViewTree = ometa::ViewTree<std::string_view>;

int main(int argc, char* argv[]) {

	start := "abc";

	auto code = R"(abc)";

	auto result = start.parse(code);
	if (result) {
		std::cout << *result << "\n";
		return EXIT_SUCCESS;
	}
	else {
		std::cout << "fail\n";
		return EXIT_FAILURE;
	}
}

EOF
fi
echo "1) implement the parser for the new syntax in the old syntax (bridge parser)"
echo "2) implement the parser for the new syntax in the new syntax"
echo "3) adapt test.ometa and calculator.ometa"
echo "4) run validate_update.sh"
echo "5) run apply_update.sh"
echo "\e[31m6) delete update folder, don't reuse for next update (bridge will probably no longer work)\e[0m"
