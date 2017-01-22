# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# define additional directories containing header files
INCLUDES = -I/usr/local/include

# define additional library paths
LFLAGS = -L/usr/local/lib

# define any libraries to link into executable
LIBS = -ldill -ldsock -lpthread

# subdirectories
SOURCES = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SOURCES))

all: $(PROGS)

%: %.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS)
clean:
	$(RM) $(PROGS)
