#!/bin/sh

. "posixshell/check_strlcpy.sh"
. "posixshell/check_strlcat.sh"

if [ $# -ne 1 ]; then
	echo "bogus number of args"
	exit 1
fi

FUNCS_YESNO_HDR=$1
shift

cat /dev/null >"${FUNCS_YESNO_HDR}"

check_strlcpy "${FUNCS_YESNO_HDR}"
check_strlcat "${FUNCS_YESNO_HDR}"

if [ ! -r "${FUNCS_YESNO_HDR}" ]; then
	echo "fatal: error creating ${FUNCS_YESNO_HDR}"
	exit 1
fi
