


#./iio-capture -n lab-baylibre-acme.local iio:device1

set -x 

./iio-probe-start 1 aaabbbcccdddeee-0 &
sleep 10

./iio-probe-stop 1 aaabbbcccdddeee




