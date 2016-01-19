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
	 "voltage0_max":	 "2935.00",
	 "voltage1_max":	 "5125.00",
	 "power2_max":	 "1525.00",
	 "power2_avg":	 "1525.00",
	 "power2_min":	 "1525.00",
	 "energy":	 "821.89",
	 "current3_max":	 "294.00",
	 "current3_min":	 "293.00",
```

