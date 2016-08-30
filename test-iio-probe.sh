#!/bin/bash
set -x

IN_ADDR=$1
IN_PROBE=$2

FILENAME="iio-probe/test"

if [ -z "${IN_ADDR}" ];then
	echo "Please provide an acme adress"
	exit 2
fi
if [ -z "${IN_PROBE}" ];then
	echo "Please provide a probe number"
	exit 2
fi
 
if [ -f /tmp/${FILENAME} ]; then
    rm -rf `dirname ${FILENAME}`
fi

./iio-probe-start ${IN_ADDR} ${IN_PROBE} /tmp/${FILENAME} &

sleep 10

./iio-probe-stop --test ${IN_PROBE} /tmp/${FILENAME}

