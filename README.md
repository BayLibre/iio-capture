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

## LAVA Test Case Signal Format ##

By default, iio-capture will output the metrics as expected by LAVA:

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ iio-capture -n lab-baylibre-acme.local iio:device1
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=vbus_max UNIT=mV MEASUREMENT=70.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=Energy UNIT=mJ MEASUREMENT=00.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=power_min UNIT=mW MEASUREMENT=00.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=power_max UNIT=mW MEASUREMENT=00.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=power_avg UNIT=mW MEASUREMENT=00.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=current_min UNIT=mA MEASUREMENT=00.00>
<LAVA_TESTCASE_SIGNAL TEST_CASE_ID=current_max UNIT=mA MEASUREMENT=-1.00>
```

## One-line output ##

Alternatively, a short form of the output can be requested with option -o/--one-line

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ iio-capture -o -n lab-baylibre-acme.local iio:device1
vmax=70.00 energy= 0.00 pmax= 0.00 pavg= 0.00 pmin= 0.00 cmax= 1.00 cmin= 0.00
```

