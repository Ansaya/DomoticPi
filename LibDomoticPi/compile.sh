#!/bin/bash

function clean_dir {
	if [ -d "$1" ]; then	
		rm -r "$1"
	fi
}

function compile {
	if [ -d "$1" ]; then
		cd "$1" || exit;
		make -j4;
	else
		mkdir "$1";
		cd "$1" || exit;
		cmake -DCMAKE_BUILD_TYPE="$1" ..;
		compile "$1";
	fi
}

if [ "$1" == "clean" ]; then
	clean_dir "$2"
else
	compile "$1"
fi
