# Defining compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Library flags
LFLAGS = -lm

all: lab3

lab3: lab3.o stats_functions.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

# CMD for cleaning
.PHONY: clean
clean:
	rm *.o