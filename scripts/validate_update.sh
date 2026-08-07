set -e
if [ ! -e "build/ometa-cpp" ]; then
    echo "building ometa-cpp (old parser for old syntax from old source)..."
    sh scripts/build_ometa-cpp.sh
fi

echo "building bridge parser to understand new syntax, written in old syntax with old includes..."
time ./build.py -o build/bridge_parser \
    --ometa-include update/include \
    --cpp build/bridge_parser.ometa.cpp \
    update/bridge_parser.ometa

echo "building new parser from new syntax and new includes using bridge parser..."
time ./build.py -o build/new_parser \
    --transpiler build/bridge_parser \
    --ometa-include update/include \
    --cpp build/new_parser.ometa.cpp \
    update/new_parser.ometa

echo "building new parser from new syntax and new includes using itself..."
time ./build.py -o build/new_parser_self \
    --transpiler build/new_parser \
    --ometa-include update/include \
    --cpp build/new_parser_self.ometa.cpp \
    update/new_parser.ometa

echo "comparing output from bridge parser and new parser..."
set +e
diff -s build/new_parser.ometa.cpp \
    build/new_parser_self.ometa.cpp
set -e

echo "building tests with new parser..."
./build.py -o build/test_new_parser \
    --transpiler build/new_parser \
    --ometa-include update/include \
    --cpp build/test.ometa.cpp \
    update/test.ometa

echo "running tests..."
./build/test_new_parser

echo "building calculator with new parser..."
./build.py -o build/calculator_new_parser \
    --transpiler build/new_parser \
    --ometa-include update/include \
    --cpp build/calculator.ometa.cpp \
    update/calculator.ometa

echo "running calculator..."
./build/calculator_new_parser "5+4*3^(2+1-0)"