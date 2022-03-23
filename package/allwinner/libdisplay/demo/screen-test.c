/*
 * screent test base on framebuffer device
 * Copyright (C) 2015-2018 AllwinnerTech
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen-test.h"
#include "testcase.h"
#include "framebuffer.h"
#include "virtual.h"

struct test_config {
	const char *testcase_name;
	const char *params;
	const char *platform;
	const char *fbdevicenode;
	int automode;
	int duration;
};

static struct test_config _config;

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s [-o device] [-n name] [-p params] [-a] [-t sec] [-h]\n", name);
	fprintf(stderr, "   -o: Output device(framebuffer/layer/virtual), default: virtual\n");
	fprintf(stderr, "   -n: Specifies the testcase name\n");
	fprintf(stderr, "   -p: Optional params for testcase\n");
	fprintf(stderr, "   -a: Auto test mode, run all testcase with default params\n");
	fprintf(stderr, "   -t: Duration for each testcase\n");
	fprintf(stderr, "   -h: Print this message\n");

	dump_available_testcase();
}

static void parse_opt(int argc, char **argv)
{
	int c;

	_config.params = NULL;
	_config.platform = "virtual";
	_config.fbdevicenode = NULL;
	_config.testcase_name = "colorbar";
	_config.automode = 0;
	_config.duration = 8;

	do {
		c = getopt(argc, argv, "o:d:n:p:at:h");
		if (c == EOF)
			break;
		switch (c) {
		case 'o':
			_config.platform = optarg;
			break;
		case 'd':
			_config.fbdevicenode = optarg;
			break;
		case 'n':
			_config.testcase_name = optarg;
			break;
		case 'p':
			_config.params = optarg;
			break;
		case 'a':
			_config.automode = 1;
			break;
		case 't':
			if (optarg)
				_config.duration = strtoul(optarg, NULL, 0);
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(1);
		}
	} while(1);

	if (optind != argc) {
		usage(argv[0]);
		exit(1);
	}

	fprintf(stdout, "     platform: %s\n", _config.platform);
	fprintf(stdout, "testcase_name: %s\n", _config.testcase_name);
	fprintf(stdout, "       params: %s\n", _config.params);
	fprintf(stdout, "     automode: %d\n", _config.automode);
	fprintf(stdout, "     duration: %d\n", _config.duration);
}

int main(int argc, char **argv)
{
	struct canvas *canvas;
	struct testcase_info *info;

	parse_opt(argc, argv);

	canvas = create_canvas(_config.platform, _config.fbdevicenode);
	if (!canvas) {
		fprintf(stderr, "create canvas failed\n");
		exit(1);
	}

	info = (struct testcase_info *)search_testcase_by_name(_config.testcase_name);
	if (info) {
		info->entry(canvas, _config.params);
	}

	destroy_canvas(canvas);

	return 0;
}
