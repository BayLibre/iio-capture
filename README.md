# IIO Capture tool #

based on iio-readdev (Paul Cerceuil)

- capture IIO buffer stream
- compute avg, mean, max of each channel
- compute energy for power channel
- listen to sigaction SIGTERM to stop record


# WIP: test output #

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ ./iio-capture -n baylibre-acme.local iio:device1 & 
[1] 10667

powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ kill -2 10667 
min[-1675414144] = 2930.000000 mSI
max[-1740703376] = 2937.500000 mSI
avg[-1740703376] = 6.250000 mSI
min[-1740703376] = 5122.500000 mSI
max[-1740703376] = 5125.000000 mSI
avg[-1740703376] = 1.562500 mSI
min[-1740703376] = 1525.000000 mSI
max[-1740703376] = 1525.000000 mSI
avg[-1740703376] = 625.000000 mSI
min[-1740703376] = 293.000000 mSI
max[-1740703376] = 294.000000 mSI
avg[-1740703376] = 1.000000 mSI
min[-1740703376] = 65535.000000 mSI
max[-1740703376] = 0.000000 mSI
avg[-1740703376] = 1.000000 mSI
```

