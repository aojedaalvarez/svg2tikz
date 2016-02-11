#
# Makefile
#


# compiler to use
CC = gcc

# flags to pass compiler
CFLAGS = -std=c99

# name for executable
EXE = svg2tikz

# space-separated list of header files
HDRS = process.h totikz.h

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -lm

# space-separated list of source files
SRCS = svg2tikz.c process.c totikz.c

# automatically generated list of object files
OBJS = $(SRCS:.c=.o)


# default target
$(EXE): $(OBJS) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# dependencies 
$(OBJS): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXE) *.o
