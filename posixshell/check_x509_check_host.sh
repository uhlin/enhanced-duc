#
# Check whether X509_check_host() exists
#
# Copyright (c) 2023 Markus Uhlin. All rights reserved.
#

check_x509_check_host () {
	local _tmpfile _srcfile _out

	printf "creating temp file..."
	_tmpfile=$(mktemp) || { echo "error"; exit 1; }
	echo "ok"

	_srcfile="${_tmpfile}.c"
	_out="${_tmpfile}.out"
	cat <<EOF >"$_srcfile"
#include <openssl/x509v3.h>

int
main(void)
{
	if (X509_check_host(NULL, "", 0, 0, NULL) == -1)
		return 1;
	return 0;
}
EOF

	if [ ! -f "$_srcfile" ]; then
		echo "failed to create $_srcfile"
		exit 1
	fi

	printf "checking for X509_check_host()..."

	local libressl_pkg_config_path="/usr/local/opt/libressl/lib/pkgconfig"

	if [ -d ${libressl_pkg_config_path} ]; then
		export PKG_CONFIG_PATH="${libressl_pkg_config_path}"
	fi

	local crypto_cflags="$(pkg-config --cflags libcrypto)"
	local crypto_libs="$(pkg-config --libs libcrypto)"

	${CC} ${CFLAGS} ${crypto_cflags} -Werror "$_srcfile" -o "$_out" \
	    ${LDFLAGS} ${crypto_libs} >/dev/null 2>&1

	if [ $? -eq 0 ]; then
		echo "yes"
		cat <<EOF >>${1}
#define HAVE_X509_CHECK_HOST 1
EOF
	else
		echo "no"
		cat <<EOF >>${1}
#define HAVE_X509_CHECK_HOST 0
EOF
	fi

	echo "cleaning..."
	test -f "$_tmpfile" && rm -f "$_tmpfile"
	test -f "$_srcfile" && rm -f "$_srcfile"
	test -f "$_out" && rm -f "$_out"
}
