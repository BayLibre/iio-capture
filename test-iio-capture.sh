#!/bin/bash

iio-capture --csv --fout /temp/delme -n lab-baylibre-acme.local iio:device$1

