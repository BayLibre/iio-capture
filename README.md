# IIO Capture tool #

based on iio-readdev (Paul Cerceuil)

- capture IIO buffer stream
- compute avg, mean, max of each channel
- compute energy for power channel
- listen to sigaction SIGTERM to stop record

Note: This is slightly hardcoded for the [BayLibre ACME](http://baylibre.com/acme) board at the moment.

This is focused on integration with LAVA for the PowerCI Power Monitoring Continuous Integration solution.

# Usage #

* call iio-capture with the IP address of the ACME board, and device ID matching the probe.
* Send a TERM signal to stop recording.
* The user may chose the data stream to be recorded as a binary or Coma Separated Value (CSV) file.

**Warning**: the iio devices are numbered in the order of discovery, meaning that if the first used slot on the cape is marked "PROBE3", iio:device0 will matche PROBE3 etc...The markings on the Cape are one-based, whilst the iio:deviceX numbering is zero-based.

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

### Example of LAVA measurement output in job log

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

### One-line ###

Alternatively, a short form of the output can be requested with option **'-o/--one-line'**

```
powerci@lava-baylibre:~/POWERCI/SRC/iio-capture$ iio-capture -o -n lab-baylibre-acme.local iio:device1
vmax=70.00 energy=0.00 pmax=0.00 pavg=0.00 pmin=0.00 cmax=1.00 cmin=0.00
```

### Energy-only ###

To reduce output, energy only can be printed with option **'-e, --energy-only'** as :

```
<LAVA_SIGNAL_TESTCASE TEST_CASE_ID=Energy RESULT=pass UNITS=mJ MEASUREMENT=00.00>
```
or
```
energy=0.00
```

## Recording to File ##

Option **'-f, --fout'** will make iio-capture to record the streamed date into a file, by default binary.
It's contents can be easily checked with 'od -x' in the case of the ina226 sensors used in BayLibre ACME RevB.

Alternatively, a CSV file can be generated, by adding option **'-c, --csv'**

# Building #

Please refer to https://wiki.analog.com/resources/tools-software/linux-software/libiio as for pre-requisites:

* package and library installation in order to build libiio
* building and installing libiio

# LAVA / PowerCI flow integration #

## LAVA Hooks ##

iio-capture is currently called from the LAVA dispatcher upon entry of a **lava-command** dispatcher action using the
script *iio-probe-start* in the git. The hooks are declared for each device under test (DUT) in the conf files as known by the LAVA dispatcher.

e.g in panda-es_0.conf:

```
lava-dispatcher.local:~$ cat /etc/lava-dispatcher/devices/panda-es_0.conf
device_type = panda-es
hostname = panda-es_0
connection_command = telnet localhost 2001

host_hook_enter_command = iio-probe-start lab-baylibre-acme.local 1
host_hook_exit_command = iio-probe-stop 1

hard_reset_command = ssh -t root@lab-baylibre-acme.local dut-hard-reset 2 &
power_off_cmd = ssh -t root@lab-baylibre-acme.local dut-switch-off 2 &
bootloader_prompt = =>
```

### iio-probe-start ####

This will take the following arguments: 

* arg1 = probe number, zero-based
* arg2 = file name for attached contents, infered from the analyzer-uuid meta in the LAVA job. 

iio-capture invocation upon entry in *lava-command*:
```
iio-capture --csv --fout $2 -n lab-baylibre-acme.local iio:device$1 > /tmp/stats-$1 &

echo "text/csv" > $2.mimetype
```

### iio-probe-stop ###

Once the dispatcher action is complete, a second _exit_ hook calls the script *iio-probe-stop*, this scrpt simply send SIGTERM to iio-capture to tell the end of the record session, and uploads the results to the PowerCI attachement folder.

