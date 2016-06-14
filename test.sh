
if [ -z "$1" ];then
	echo "Please provide an acme adress"
	exit 2
fi
if [ -z "$2" ];then
	echo "Please provide a file name"
	exit 2
fi


set -x 

./iio-probe-start $1 1 /tmp/$2-0 &
sleep 10

./iio-probe-stop 1 /tmp/$2-0

