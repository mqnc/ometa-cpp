set -e

cd build
cmake ..

make old_parser_in_old_syntax

./old_parser_in_old_syntax \
	../src/new_parser_in_old_syntax.ometa \
	../src/new_parser_in_old_syntax.ometa.cpp

make new_parser_in_old_syntax

./new_parser_in_old_syntax \
	../src/new_parser_in_new_syntax.ometa \
	../src/new_parser_in_new_syntax.ometa.cpp

make new_parser_in_new_syntax

./new_parser_in_new_syntax \
	../src/new_parser_in_new_syntax.ometa \
	../src/verify_new_parser_in_new_syntax.ometa.cpp

diff -s \
	../src/new_parser_in_new_syntax.ometa.cpp \
	../src/verify_new_parser_in_new_syntax.ometa.cpp
