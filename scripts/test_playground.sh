set -e

./build.py -o build/playground \
    --transpiler build/new_parser \
    --cpp build/playground.ometa.cpp \
    --ometa-include update/include \
    "$@" update/playground.ometa
./build/playground