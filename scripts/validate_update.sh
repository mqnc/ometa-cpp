set -e
if [ ! -e "build/ometa-cpp" ]; then
    echo "building ometa-cpp (old parser from old source)..."
    sh scripts/build_ometa-cpp.sh
fi

echo "building new parser from old syntax..."
./build.py -o build/new_parser_from_old_syntax \
    --cpp build/new_parser_from_old_syntax.ometa.cpp \
    update/new_parser_from_old_syntax.ometa

echo "building new parser from new syntax..."
./build.py -o build/new_parser_from_new_syntax \
    --transpiler build/new_parser_from_old_syntax \
    --cpp build/new_parser_from_new_syntax.ometa.cpp \
    update/new_parser_from_new_syntax.ometa

echo "comparing new and old transpiled parsers..."
diff -s build/new_parser_from_old_syntax.ometa.cpp \
    build/new_parser_from_new_syntax.ometa.cpp

echo "building tests with new parser..."
./build.py -o build/test_new_parser \
    --transpiler build/new_parser_from_new_syntax \
    --cpp build/test.ometa.cpp \
    examples/test.ometa

echo "running tests..."
./build/test_new_parser
