#
# iniparser tests Makefile
#

CC := ~/workspace/aw1689/lichee/out/external-toolchain/gcc-aarch64/bin/aarch64-linux-gnu-gcc
CFLAGS  = -g -I../src
LFLAGS  = -L.. -liniparser
AR := ~/workspace/aw1689/lichee/out/external-toolchain/gcc-aarch64/bin/aarch64-linux-gnu-ar
ARFLAGS = rcv
RM      = rm -f


default: all

all: iniexample parse

iniexample: iniexample.c
	$(CC) $(CFLAGS) -o iniexample iniexample.c -I../src -L.. -liniparser

parse: parse.c
	$(CC) $(CFLAGS) -o parse parse.c -I../src -L.. -liniparser

clean veryclean:
	$(RM) iniexample example.ini parse



