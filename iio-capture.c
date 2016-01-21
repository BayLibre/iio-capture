/*
 * libiio - Library for interfacing industrial I/O (IIO) devices
 *
 * Copyright (C) 2016 BayLibre SAS, parts Analog Devices, Inc.
 *
 * Author: Paul Cercueil <paul.cercueil@analog.com>
 * Marc Titinger <mtitinger@baylibre.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * */

#include <getopt.h>
#include <iio.h>
#include <signal.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MY_NAME "iio-capture"

#define SAMPLES_PER_READ 128
#define OVER_SAMPLING 1
#define DEFAULT_FREQ_HZ  455

static long long sampling_freq;

#define MAX_CHANNELS 10

/*
 * Trivial helper structure for a scan_element.
 */
struct my_channel {
	char label[128];
	const char *unit;
	int min;
	int max;
	double scale;
	double avg;		/* moving average */
	double energy;		/* linear integration over time */
	int frequency;		/* sampling freq of channel */
	long long count;	/* sample count */

#define HAS_NRJ	0x01
#define HAS_MIN 0x02
#define HAS_AVG 0x04
#define HAS_MAX 0x08
	unsigned int flags;
};

static unsigned int nb_channels;
static unsigned int nb_samples;
static struct my_channel my_chn[MAX_CHANNELS];

static const struct option options[] = {
	{"help", no_argument, 0, 'h'},
	{"network", required_argument, 0, 'n'},
	{"usb", required_argument, 0, 'u'},
	{"trigger", required_argument, 0, 't'},
	{"buffer-size", required_argument, 0, 'b'},
	{0, 0, 0, 0},
};

static const char *options_descriptions[] = {
	"Show this help and quit.",
	"Use the network backend with the provided hostname.",
	"Use the USB backend with the device that matches the given VID/PID.",
	"Use the specified trigger.",
	"Size of the capture buffer. Default is 256.",
};

static void usage(void)
{
	unsigned int i;

	printf("Usage:\n\t" MY_NAME " [-n <hostname>] [-u <vid>:<pid>] "
	       "[-t <trigger>] [-b <buffer-size>] "
	       "<iio_device> [<channel> ...]\n\nOptions:\n");
	for (i = 0; options[i].name; i++)
		printf("\t-%c, --%s\n\t\t\t%s\n",
		       options[i].val, options[i].name,
		       options_descriptions[i]);
}

static struct iio_context *ctx;
struct iio_buffer *buffer;
static const char *trigger_name = NULL;

static bool app_running = true;
static int exit_code = EXIT_SUCCESS;

/* Create report compatible with lava-report.py form */
static void channel_report(int i)
{
	if (my_chn[i].flags && !my_chn[i].count) {
		printf("%s: no sample acquired, ", my_chn[i].label);
		return;
	}

	if (my_chn[i].flags & HAS_MAX)
		printf("%cmax=%5.2f ", my_chn[i].label[0],
		       my_chn[i].max * my_chn[i].scale);

	if (my_chn[i].flags & HAS_AVG)
		printf("%cavg=%5.2f ", my_chn[i].label[0],
		       my_chn[i].avg * my_chn[i].scale);

	if (my_chn[i].flags & HAS_MIN)
		printf("%cmin=%5.2f ", my_chn[i].label[0],
		       my_chn[i].min * my_chn[i].scale);

	/*only power channel is expected to have energy */
	if (my_chn[i].flags & HAS_NRJ)
		printf("energy=%5.2f ",
		       my_chn[i].energy / sampling_freq);
}

static void quit_all(int sig)
{
	int i;
	exit_code = sig;
	app_running = false;

	for (i = 0; i < nb_channels; i++)
		channel_report(i);
}

static void set_handler(int signal_nb, void (*handler) (int))
{
#ifdef _WIN32
	signal(signal_nb, handler);
#else
	struct sigaction sig;
	sigaction(signal_nb, NULL, &sig);
	sig.sa_handler = handler;
	sigaction(signal_nb, &sig, NULL);
#endif
}

static struct iio_device *get_device(const struct iio_context *ctx,
				     const char *id)
{

	unsigned int i, nb_devices = iio_context_get_devices_count(ctx);
	struct iio_device *device;

	for (i = 0; i < nb_devices; i++) {
		const char *name;
		device = iio_context_get_device(ctx, i);
		name = iio_device_get_name(device);
		if (name && !strcmp(name, id))
			break;
		if (!strcmp(id, iio_device_get_id(device)))
			break;
	}

	if (i < nb_devices)
		return device;

	fprintf(stderr, "Device %s not found\n", id);
	return NULL;
}

static inline int chan_index(const struct iio_channel *chn)
{
	const char *id = iio_channel_get_id(chn);

	int idx = id[strlen(id) - 1];

	if ((idx > '9') || (idx < '0'))
		return -1;

	return idx - '0';
}

static ssize_t print_sample(const struct iio_channel *chn,
			    void *buf, size_t len, void *d)
{
//      fwrite(buf, 1, len, stdout);
	int val = (int)*(short *)buf;
	int vabs = 0;
	int i;

	i = chan_index(chn);
	if (i < 0)
		return len;

	vabs = abs(val);

	if (abs(my_chn[i].min) > vabs)
		my_chn[i].min = val;

	if (abs(my_chn[i].max) < vabs)
		my_chn[i].max = val;

	my_chn[i].count++;

	if (my_chn[i].flags & HAS_AVG)
		my_chn[i].avg +=
		    ((double)(val) - my_chn[i].avg) / my_chn[i].count;

	if (my_chn[i].flags & HAS_NRJ)
		my_chn[i].energy += (double)val;

	nb_samples++;

	return len;
}

static void init_ina2xx_channels(struct iio_device *dev)
{
	int i;
	char buf[1024];
	struct iio_channel *ch;

	if (strcmp(iio_device_get_name(dev),"ina226")) {
		fprintf(stderr, "Unknown device %s\n", iio_device_get_name(dev));
		exit(-1);
	}
		
	nb_channels = iio_device_get_channels_count(dev);

	nb_samples = 0;

	/* FIXME: dyn alloc */
	assert(nb_samples <= MAX_CHANNELS);

	for (i = 0; i < nb_channels; i++) {
		const char* id;
		ch = iio_device_get_channel(dev, i);

		my_chn[i].min = 0xffff;
		my_chn[i].max = 0;
		my_chn[i].flags = 0;

		id = iio_channel_get_id(ch);

		/* skip timestamp, and vshunt, cheesy fashon */
		if (!strcmp(id, "timestamp") || !strcmp(id, "voltage0"))
			continue;

		if (iio_channel_attr_read(ch, "scale", buf, sizeof(buf)) >= 0)
			my_chn[i].scale = atof(buf);
		else
			my_chn[i].scale = 1.0;

		my_chn[i].flags = HAS_MAX;

		if (!strncmp(id, "power", 5)) {
			my_chn[i].flags |= HAS_MIN|HAS_AVG;
			if (sampling_freq)
				my_chn[i].flags |= HAS_NRJ;
			/*trim the label to remove the */
			strcpy(my_chn[i].label, "power");
	
		} else if (!strncmp(id, "current", 6)) {
			my_chn[i].flags |= HAS_MIN;
			
                        /*trim the label to remove the */
                        strcpy(my_chn[i].label, "current");

                } else
			strcpy(my_chn[i].label, "voltage");

		my_chn[i].count = 0;
	}
}

int main(int argc, char **argv)
{
	unsigned int i;
	unsigned int buffer_size = SAMPLES_PER_READ;
	int c, option_index = 0, arg_index = 0, ip_index = 0, device_index = 0;
	struct iio_device *dev;
	char temp[1024];

	while ((c = getopt_long(argc, argv, "+hn:u:t:b:",
				options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			usage();
			return EXIT_SUCCESS;
		case 'n':
			arg_index += 2;
			ip_index = arg_index;
			break;
		case 'u':
			arg_index += 2;
			device_index = arg_index;
			break;
		case 't':
			arg_index += 2;
			trigger_name = argv[arg_index];
			break;
		case 'b':
			arg_index += 2;
			buffer_size = atoi(argv[arg_index]);
			break;
		case '?':
			return EXIT_FAILURE;
		}
	}

	if (arg_index + 1 >= argc) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		usage();
		return EXIT_FAILURE;
	}

	if (ip_index) {
		ctx = iio_create_network_context(argv[ip_index]);
	} else if (device_index) {
		char *ptr = argv[device_index], *end;
		long vid, pid;

		vid = strtol(ptr, &end, 0);
		if (ptr == end || *end != ':') {
			fprintf(stderr, "Invalid VID/PID\n");
			return EXIT_FAILURE;
		}

		ptr = end + 1;
		pid = strtol(ptr, &end, 0);
		if (ptr == end) {
			fprintf(stderr, "Invalid VID/PID\n");
			return EXIT_FAILURE;
		}

		ctx = iio_create_usb_context((unsigned short)vid,
					     (unsigned short)pid);
	} else {
		ctx = iio_create_default_context();
	}

	if (!ctx) {
		fprintf(stderr, "Unable to create IIO context\n");
		return EXIT_FAILURE;
	}
#ifndef _WIN32
	set_handler(SIGHUP, &quit_all);
#endif
	set_handler(SIGINT, &quit_all);
	set_handler(SIGSEGV, &quit_all);
	set_handler(SIGTERM, &quit_all);

	dev = get_device(ctx, argv[arg_index + 1]);
	if (!dev) {
		iio_context_destroy(ctx);
		return EXIT_FAILURE;
	}

	if (trigger_name) {
		struct iio_device *trigger = get_device(ctx, trigger_name);
		if (!trigger) {
			iio_context_destroy(ctx);
			return EXIT_FAILURE;
		}

		if (!iio_device_is_trigger(trigger)) {
			fprintf(stderr, "Specified device is not a trigger\n");
			iio_context_destroy(ctx);
			return EXIT_FAILURE;
		}

		/*
		 * Fixed rate for now. Try new ABI first,
		 * fail gracefully to remain compatible.
		 */
		if (iio_device_attr_write_longlong(trigger,
						   "sampling_frequency",
						   DEFAULT_FREQ_HZ) < 0)
			iio_device_attr_write_longlong(trigger, "frequency",
						       DEFAULT_FREQ_HZ);

		iio_device_set_trigger(dev, trigger);

	} else {
	
		sprintf(temp, "%d", OVER_SAMPLING);

		c = iio_device_attr_write(dev, "in_oversampling_ratio", temp);
		if (c < 0) {
			fprintf(stderr, "Unsupported write attribute 'in_oversampling_ratio'");
			exit(-1);
		}

		/* store actual sampling freq */
		c = iio_device_attr_read(dev, "in_sampling_frequency", temp, 1024);
		if (c)
			sampling_freq = atoi(temp);
	}

	init_ina2xx_channels(dev);

	if (argc == arg_index + 2) {
		/* Enable all channels */
		for (i = 0; i < nb_channels; i++)
			iio_channel_enable(iio_device_get_channel(dev, i));
	} else {
		for (i = 0; i < nb_channels; i++) {
			unsigned int j;
			struct iio_channel *ch = iio_device_get_channel(dev, i);
			for (j = arg_index + 2; j < argc; j++) {
				const char *n = iio_channel_get_name(ch);
				if (!strcmp(argv[j], iio_channel_get_id(ch)) ||
				    (n && !strcmp(n, argv[j])))
					iio_channel_enable(ch);
			}
		}
	}

	buffer = iio_device_create_buffer(dev, buffer_size, false);
	if (!buffer) {
		fprintf(stderr, "Unable to allocate buffer\n");
		iio_context_destroy(ctx);
		return EXIT_FAILURE;
	}

	while (app_running) {
		int ret = iio_buffer_refill(buffer);
		if (ret < 0) {
			fprintf(stderr, "Unable to refill buffer: %s\n",
				strerror(-ret));
			break;
		}
		iio_buffer_foreach_sample(buffer, print_sample, NULL);
	}

	iio_buffer_destroy(buffer);
	iio_context_destroy(ctx);
	return exit_code;
}
