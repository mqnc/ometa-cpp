cd build
cmake ..

make old_parser_in_old_syntax

./old_parser_in_old_syntax new_parser_in_old_syntax.ometa new_parser_in_old_syntax.ometa.cpp

make new_parser_in_old_syntax

./new_parser_in_old_syntax new_parser_in_new_syntax.ometa new_parser_in_new_syntax.ometa.cpp

make new_parser_in_new_syntax

./new_parser_in_new_syntax new_parser_in_new_syntax.ometa verify_new_parser_in_new_syntax.ometa.cpp

diff -s new_parser_in_new_syntax.ometa.cpp verify_new_parser_in_new_syntax.ometa.cpp