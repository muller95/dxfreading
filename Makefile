CC = gcc
CCFLAGS = -lm -g `pkg-config --cflags --libs gtk+-3.0`
SRCS = main.c nestapi_core.c dxf_nest.c nfp_nest_test.c dxf_work_functions.c dxf_geometry.c cross_check_funcs.c nestapi_core_int.c nfp_nest_int.c dxf_geometry_int.c dxf_work_functions_int.c cross_check_funcs_int.c
OBJS = $(SRCS:.c=.o)

BINS = test

all:
	$(CC) $^ $(CCFLAGS) -o $(BINS) $(SRCS)
