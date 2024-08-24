set -e

cd build
cmake ..

echo "building old_parser_in_old_syntax"
make old_parser_in_old_syntax

echo "transpiling new_parser_in_old_syntax using old parser"
./old_parser_in_old_syntax \
	../src/new_parser_in_old_syntax.ometa \
	../src/new_parser_in_old_syntax.ometa.cpp

echo "building new_parser_in_old_syntax"
make new_parser_in_old_syntax

echo "transpiling new_parser_in_new_syntax using new parser that was written in old syntax"
./new_parser_in_old_syntax \
	../src/new_parser_in_new_syntax.ometa \
	../src/new_parser_in_new_syntax.ometa.cpp

echo "building new_parser_in_new_syntax"
make new_parser_in_new_syntax

echo "transpiling new_parser_in_new_syntax using new parser that was written in new syntax"
./new_parser_in_new_syntax \
	../src/new_parser_in_new_syntax.ometa \
	../src/verify_new_parser_in_new_syntax.ometa.cpp

echo "comparing results"
diff -s \
	../src/new_parser_in_new_syntax.ometa.cpp \
	../src/verify_new_parser_in_new_syntax.ometa.cpp
