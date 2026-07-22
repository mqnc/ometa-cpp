set -e
if [ ! -e "build/ometa-cpp" ]; then
    echo "building ometa-cpp (old parser from old source)..."
    sh scripts/build_ometa-cpp.sh
fi

echo "building new parser from old source..."
./build.py -o build/new_parser_from_old_source \
    --cpp build/new_parser_from_old_source.ometa.cpp \
    update/new_parser_from_old_source.ometa

echo "building new parser from new source..."
./build.py -o build/new_parser_from_new_source \
    --transpiler build/new_parser_from_old_source \
    --cpp build/new_parser_from_new_source.ometa.cpp \
    update/new_parser_from_new_source.ometa

echo "comparing new and old transpiled parsers..."
diff -s build/new_parser_from_old_source.ometa.cpp \
    build/new_parser_from_new_source.ometa.cpp

echo "building tests with new parser..."
./build.py -o build/test_new_parser \
    --transpiler build/new_parser_from_new_source \
    --cpp build/test.ometa.cpp \
    examples/test.ometa

echo "running tests..."
./build/test_new_parser
