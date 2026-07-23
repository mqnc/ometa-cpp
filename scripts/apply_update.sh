set -e
sh scripts/validate_update.sh
echo "overwriting old parser source, includes, transpiled and executable with new parser..."
cp -f update/new_parser_from_new_syntax.ometa examples/ometa.ometa
cp -f build/new_parser_from_new_syntax.ometa.cpp ometa.cpp
cp -f build/new_parser_from_new_syntax build/ometa-cpp
rm -rf include
cp -r update/include include
