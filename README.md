# IIO Capture tool #

based on iio-readdev (Paul Cerceuil)

- capture IIO buffer stream
- compute avg, mean, max of each channel
- compute energy for power channel
- listen to sigaction SIGTERM to stop record

Note: This is slightly hardcoded for the ACME board ATM.

# Usage #

* call iio-capture with the IP address of the ACME board, and device ID matching the probe. 
* Send a TERM signal to stop recording.

# Output #

All units are milli-SI

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ ./iio-capture -n lab-baylibre-acme.local iio:device1 & 
[1] 10667

powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ kill -2 10667 
	 "voltage_max":	 5110.00,
	 "power_max":	 2475.00,
	 "power_avg":	 1981.45,
	 "power_min":	 1925.00,
	 "energy":	 533.95,
	 "current_max":	 484.00,
	 "current_min":	 378.00,
```

