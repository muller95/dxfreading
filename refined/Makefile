CC = gcc
CCFLAGS = -Wall -lm -g -lpthread `pkg-config --cflags --libs gtk+-3.0`
SRCS = main.c dxf_file.c
OBJS = $(SRCS:.c=.o)

BINS = test

all:
	$(CC) $^ $(CCFLAGS) -o $(BINS) $(SRCS)
