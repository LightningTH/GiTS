RELEASE = 1

SVCNAME = dynapwn

CC = gcc
CFLAGS = -g -Wall -Wshadow -Wextra $(EXTRAFLAGS) -m32 -O2 -pie
INCLUDEDIR = ../../../lib
LIBS = -lz

ifeq ($(RELEASE),1)
DEFINES = -DRELEASE=1
endif

ifeq ($(IPV6),1)
DEFINES += -DIPV6
ifeq ($(IPV6_DUAL_STACK),1)
DEFINES += -DIPV6_DUAL_STACK
endif
endif

# This makes it so that each c file is compiled in the directory
CFILES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(CFILES))

all: common.o $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFINES) common.o $(OBJECTS) -o $(SVCNAME) $(LIBS)
	strip -s $(SVCNAME)

common.o:
	$(CC) $(CFLAGS) $(DEFINES) -c -I$(INCLUDEDIR) $(INCLUDEDIR)/common.c

%.o:  %.c
	$(CC) $(CFLAGS) $(DEFINES) -I$(INCLUDEDIR) -c $< -o $@

install:
	setup.sh
	
clean:
	rm -rf $(SVCNAME) *.o

.PHONY: clean
