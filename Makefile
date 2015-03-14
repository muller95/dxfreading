CC = gcc
CCFLAGS = -lm -g -lpthread `pkg-config --cflags --libs gtk+-3.0`
SRCS = main.c nestapi_core.c dxf_nest.c nfp_nest.c nfp_nest_mt.c dxf_work_functions.c dxf_geometry.c cross_check_funcs.c
OBJS = $(SRCS:.c=.o)

BINS = test

all:
	$(CC) $^ $(CCFLAGS) -o $(BINS) $(SRCS)
