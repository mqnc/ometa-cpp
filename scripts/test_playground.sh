set -e

./build.py -o build/playground \
    --cpp build/playground.ometa.cpp \
    --ometa-include update/include \
    "$@" update/playground.ometa
./build/playground