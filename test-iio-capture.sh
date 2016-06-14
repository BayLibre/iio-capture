#!/bin/bash

iio-capture --csv --fout /temp/delme -n $1 iio:device$2

