#!/bin/sh

PREFIX=tests/
TEST_FILES="test1.conf test2.conf test3.conf"

for conf in $TEST_FILES; do
    echo "========== ${PREFIX}${conf} =========="
    ./educ_noip -x ${PREFIX}${conf} -o
done

exit 0
