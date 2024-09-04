set -e
mkdir update
cp -r -n include update/include
cp -n examples/ometa.ometa update/new_parser_from_old_source.ometa
cp -n examples/ometa.ometa update/new_parser_from_new_source.ometa