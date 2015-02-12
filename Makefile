all:
	gcc `pkg-config --cflags gtk+-3.0` -o test main.c nestapi_core.c dxf_nest.c nfp_nest2.c dxf_work_functions.c dxf_geometry.c cross_check_funcs.c nfp_insert_funcs.c `pkg-config --libs gtk+-3.0` -lm -g

