set -e
echo "building ometa-cpp..."
sh scripts/build_ometa-cpp.sh
echo "transpiling ometa.ometa to build/ometa2.cpp..."
build/ometa-cpp examples/ometa.ometa build/ometa2.cpp
echo "comparing ometa.cpp and ometa2.cpp..."
diff -s ometa.cpp build/ometa2.cpp
