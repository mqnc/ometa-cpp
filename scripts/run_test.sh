set -e
mkdir -p build
echo "building ometa-cpp..."
sh scripts/build_ometa-cpp.sh
echo "building test..."
./build.py -o build/test examples/test.ometa
echo "running test..."
build/test
