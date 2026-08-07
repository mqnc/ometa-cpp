set -e
sh scripts/validate_update.sh
echo "overwriting old parser source, includes, transpiled, executable and tests..."
cp -f update/new_parser.ometa examples/ometa.ometa
cp -f build/new_parser.ometa.cpp ometa.cpp
cp -f build/new_parser build/ometa-cpp
rm -rf include
cp -r update/include include
cp -f update/test.ometa examples/test.ometa
cp -f update/calculator.ometa examples/calculator.ometa
echo "\e[31mNOW DELETE THE UPDATE FOLDER, DON'T REUSE IT!!!\e[0m"
