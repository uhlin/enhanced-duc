#
# Check whether strlcpy() exists
#
# Copyright (c) 2023 Markus Uhlin. All rights reserved.
#

check_strlcpy () {
	local _tmpfile _srcfile _out

	printf "creating temp file..."
	_tmpfile=$(mktemp) || { echo "error"; exit 1; }
	echo "ok"

	_srcfile="${_tmpfile}.c"
	_out="${_tmpfile}.out"
	cat <<EOF >"$_srcfile"
#include <stdio.h>
#include <string.h>

int
main(void)
{
	char buf[50] = { '\0' };

	(void) strlcpy(buf, "strlcpy works!", sizeof buf);
	return puts(buf) == EOF ? 1 : 0;
}
EOF

	if [ ! -f "$_srcfile" ]; then
		echo "failed to create $_srcfile"
		exit 1
	fi

	printf "checking for strlcpy()..."

	${CC} ${CFLAGS} -Werror "$_srcfile" -o "$_out" ${LDFLAGS} \
	    >/dev/null 2>&1

	if [ $? -eq 0 ]; then
		echo "yes"
		cat <<EOF >>${1}
#define HAVE_STRLCPY 1
EOF
	else
		echo "no"
		cat <<EOF >>${1}
#define HAVE_STRLCPY 0
EOF
	fi

	echo "cleaning..."
	test -f "$_tmpfile" && rm -f "$_tmpfile"
	test -f "$_srcfile" && rm -f "$_srcfile"
	test -f "$_out" && rm -f "$_out"
}
