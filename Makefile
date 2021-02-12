#
# Makefile for lab 6, part 2
#

CC  = gcc
CXX = g++

INCLUDES = -I
CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

LDFLAGS = -g 
LDLIBS = 

http-client: http-client.c

.PHONY: clean
clean:
	rm -f *.o *~ a.out core http-client *.html

.PHONY: all
all: clean http-client