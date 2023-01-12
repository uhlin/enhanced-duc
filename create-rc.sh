#!/bin/sh

if [ $# -ne 1 ]; then
	echo "bogus number of args"
	exit 1
fi

PREFIX=$1
shift

if [ ! -d "${PREFIX}" ]; then
	echo "bogus prefix"
	exit 1
fi

RCFILE=educ.rc

cat <<EOF >${RCFILE}
#!/bin/ksh

daemon="${PREFIX}/bin/enhanced-duc"
daemon_flags=-B

. /etc/rc.d/rc.subr

rc_reload=NO

rc_cmd \$1
EOF

if [ ! -r ${RCFILE} ]; then
	echo "fatal: error creating ${RCFILE}"
	exit 1
fi
