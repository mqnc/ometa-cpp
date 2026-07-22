set -e
echo "creating update folder and copying parser sources..."
mkdir update
cp -r --update=none include update/include
cp --update=none examples/ometa.ometa update/new_parser_from_old_syntax.ometa
cp --update=none examples/ometa.ometa update/new_parser_from_new_syntax.ometa
echo "1) implement the new parser in the old syntax"
echo "2) implement the new parser in the new syntax"
echo "3) run validate_update.sh"
echo "4) run apply_update.sh"
