#!/bin/sh

PREFIX=tests/
TEST_FILES="
test1.conf
test2.conf
test3.conf
test4.conf
test5.conf
test6.conf
"

for conf in $TEST_FILES; do
    echo "========== ${PREFIX}${conf} =========="

    if test -f "${PREFIX}${conf}"; then
	./enhanced-duc -x ${PREFIX}${conf} -o
    else
	echo "No output"
	exit 1
    fi
done
