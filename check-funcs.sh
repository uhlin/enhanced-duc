#!/bin/sh

if [ -z ${CC+x} ]; then
	CC=cc
fi

if [ -z ${CFLAGS+x} ]; then
	CFLAGS="-D_BSD_SOURCE=1 -D_DEFAULT_SOURCE=1 -D_POSIX_C_SOURCE=200809L"
fi

if [ -z ${LDFLAGS+x} ]; then
	LDFLAGS=""
fi

. "posixshell/check_strlcpy.sh"
. "posixshell/check_strlcat.sh"
. "posixshell/check_x509_check_host.sh"

if [ $# -ne 1 ]; then
	echo "bogus number of args"
	exit 1
fi

FUNCS_YESNO_HDR=$1
shift

cat /dev/null >"${FUNCS_YESNO_HDR}"

check_strlcpy "${FUNCS_YESNO_HDR}"
check_strlcat "${FUNCS_YESNO_HDR}"
check_x509_check_host "${FUNCS_YESNO_HDR}"

if [ ! -r "${FUNCS_YESNO_HDR}" ]; then
	echo "fatal: error creating ${FUNCS_YESNO_HDR}"
	exit 1
fi
