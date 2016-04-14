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
* The user maybe ask for the date stream to be recorded as a binary of Coma Separated Value file.

# Output #

All units are milli-SI, including Timestamps in milli-seconds.

## Power Stats computation at end of record session ##

### LAVA Test Case Signal Format ###

By default, iio-capture will output the metrics as expected by LAVA:

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ iio-capture -n lab-baylibre-acme.local iio:device1
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=vbus_max RESULT=pass UNITS=mV MEASUREMENT=5178.75>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=Energy RESULT=pass UNITS=mJ MEASUREMENT=677.45>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=power_min RESULT=pass UNITS=mW MEASUREMENT=275.00>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=power_max RESULT=pass UNITS=mW MEASUREMENT=3575.00>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=power_avg RESULT=pass UNITS=mW MEASUREMENT=1180.46>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=current_min RESULT=pass UNITS=mA MEASUREMENT=55.00>
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=current_max RESULT=pass UNITS=mA MEASUREMENT=693.00>
```

### Example of LAVA measurement output in log

```
111.1  ============
111.2  lava-command
111.3  ============
111.4
111.5  Test case Result Measurement
111.6  ---------------------------------------- -------- --------------------
111.7  echo 'suspend/resume' PASS
111.8  echo +10 > /sys/class/rtc/rtc0/wakealarm PASS
111.9  echo mem > /sys/power/state PASS
111.10  vbus_max PASS 5178.75 mV
111.11  energy PASS 677.45 mJ
111.12  power_min PASS 275.00 mW
111.13  power_max PASS 3575.00 mW
111.14  power_avg PASS 1180.46 mW
111.15  current_min PASS 55.00 mA
111.16  current_max PASS 693.00 mA
```

## One-line output ##

Alternatively, a short form of the output can be requested with option -o/--one-line

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ iio-capture -o -n lab-baylibre-acme.local iio:device1
vmax=70.00 energy= 0.00 pmax= 0.00 pavg= 0.00 pmin= 0.00 cmax= 1.00 cmin= 0.00
```

## Energy-Only output ##

To reduce output, energy only can be printed with option **'-e, --energy-only'** as :

```
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=Energy RESULT=pass UNITS=mJ MEASUREMENT=00.00>
```
or
```
energy= 0.00
```

# Building #

Please refer to https://wiki.analog.com/resources/tools-software/linux-software/libiio as for pre-requisites:

* package and library installation in order to build libiio
* building and installing libiio

