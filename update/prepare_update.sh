mkdir build
mkdir src

# for building the old parser
cp -n ../ometa-cpp.ometa.cpp src/old_parser.ometa.cpp

# the following two are only there so cmake does not complain
cp -n ../ometa-cpp.ometa.cpp src/new_parser_in_old_syntax.ometa.cpp
cp -n ../ometa-cpp.ometa.cpp src/new_parser_in_new_syntax.ometa.cpp

# only edit these two
cp -n ../ometa-cpp.ometa src/new_parser_in_old_syntax.ometa
cp -n ../ometa-cpp.ometa src/new_parser_in_new_syntax.ometa
