# libiio - Library for interfacing industrial I/O (IIO) devices
#
# Copyright (C) 2014 Analog Devices, Inc.
# Author: Paul Cercueil <paul.cercueil@analog.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

kversion=$(shell uname -r)

TARGETS := iio-capture

CFLAGS = -Wall -g -O1 

.PHONY: all clean

all: $(TARGETS)

iio-capture: iio-capture.o
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread -liio

install: iio-capture
	sudo install -s -v iio-capture /usr/bin
	sudo install -v iio-probe* /usr/bin

clean:
	rm -f $(TARGETS) *.o
	rm -f ./*.mimetype ./*.dat ./*.png *.csv
