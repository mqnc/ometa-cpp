#!/bin/bash

set -e

case "$1" in
	prepare)
		echo "preparing update..."
		mkdir update
		cp -r -n include update/include
		cp -n examples/ometa.ometa update/new_parser_from_old_source.ometa
		cp -n examples/ometa.ometa update/new_parser_from_new_source.ometa
		;;

	test)
		mkdir build
		cd build
		cmake ..
		make verify-update
		;;

	apply)
		mkdir build_update
		cd build_update
		cmake ..
		make verify-update
		cd ..
		rm -rf include
		cp -r update/include include
		cp -f update/new_parser_from_new_source.ometa examples/ometa.ometa
		cp -f build_update/transpiled/new_parser_from_new_source.ometa.cpp ometa.cpp
		;;

	*)
		echo "usage: $0 {prepare|test|apply}"
		exit 1
		;;
esac