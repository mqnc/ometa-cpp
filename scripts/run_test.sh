set -e
mkdir -p build
echo "building ometa-cpp..."
sh scripts/build_ometa-cpp.sh
echo "building test..."
./build.py -o build/test examples/test.ometa
echo "running test..."
build/test
echo "building calculator..."
./build.py -o build/calculator examples/calculator.ometa
echo "running calculator..."
build/calculator "5+4*3^(2+1-0)"