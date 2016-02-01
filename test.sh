if [ -z "$1" ]
then
	echo "Please provide a file name"
	exit 2
fi


set -x 

./iio-probe-start 1 /tmp/$1-0 &
sleep 10

./iio-probe-stop 1 /tmp/$1-0

