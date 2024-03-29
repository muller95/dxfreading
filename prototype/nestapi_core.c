#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nestapi_core_structs.h"
#include "dxf_geometry.h"

#define MAX_LINE_LENGTH 2048

void dxf_file_new(struct DxfFile* dxf_file, char *path, int quant);

static void read_entity(struct DxfFile *dxf_file);
static void prepare_to_read(char *path, struct DxfFile *dxf_file);
static void find_control_dots(struct DxfFile *dxf_file);
static int is_new_figure(char *line, struct DxfFile *dxf_file, int *n_types);
static int is_new_figure2(char *line, struct DxfFile *dxf_file);

struct TextPrimitive *text_primitives;
static int max_lines;
static int *str_count;

void dxf_file_new(struct DxfFile* dxf_file, char *path, int quant)
{
	int i;

	dxf_file->path = path;
	dxf_file->n_primitives = 0;
    dxf_file->how_many = quant;

	prepare_to_read(path, dxf_file);
	read_entity(dxf_file);
	find_control_dots(dxf_file);
	move_to_zero(dxf_file);
	create_polygon_jarvis(dxf_file);
	gravity_center_in_polygon(dxf_file);
}

static int is_new_figure2(char *line, struct DxfFile *dxf_file)
{
	if (strcmp(line, "LINE\r\n") == 0) {
		dxf_file->n_primitives++;
		return 1;
	}

	if (strcmp(line, "SPLINE\r\n") == 0) {
		dxf_file->n_primitives++;
		return 1;
	}	
	return 0;
}

static int is_new_figure(char *line, struct DxfFile *dxf_file, int *n_types)
{
	if (strcmp(line, "LINE\r\n") == 0) {
		dxf_file->primitives[*n_types].type = DXF_TYPE_LINE;
		*n_types += 1;
		return 1;
	} else if(strcmp(line, "SPLINE\r\n") == 0) {
		dxf_file->primitives[*n_types].type = DXF_TYPE_SPLINE;
		*n_types += 1;
		return 1;
	}
	return 0;
}


static void prepare_to_read(char *path, struct DxfFile *dxf_file)
{
	int curr_line = 0, write = 0;
	char *line;
	FILE *txt_file;

	line = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);

	txt_file = fopen(dxf_file->path, "r");
	if (!txt_file)
		printf("null\n");
	while (line = fgets(line, MAX_LINE_LENGTH, txt_file)) {
		if (strcmp(line, "ENTITIES\r\n") == 0) {
			write = 1;
		}

		is_new_figure2(line, dxf_file);

		if (write) {
			curr_line++;
		}

		if (strcmp(line, "ENDSECS\r\n") == 0 && write) {
			write = 0;
			break;
		}	
	}

	fclose(txt_file);
	max_lines = curr_line;
}

static void read_entity(struct DxfFile *dxf_file)
{
	int curr_line = 0, curr_prim = 0, write = 0, i = 0, j = 0, n_types = 0;
	char *line;
	FILE *txt_file;

	line = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	dxf_file->primitives = (struct DxfPrimitive*)malloc(sizeof(struct DxfPrimitive) * dxf_file->n_primitives);
	str_count = (int*)malloc(sizeof(int) * dxf_file->n_primitives);
	text_primitives = (struct TextPrimitive*)malloc(sizeof(struct TextPrimitive) * dxf_file->n_primitives);

	for (i = 0; i < dxf_file->n_primitives; i++) {
		text_primitives[i].lines = (char**)malloc(sizeof(char*) * max_lines);
		for (j = 0; j < max_lines; j++)
			text_primitives[i].lines[j] = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	}

	txt_file = fopen(dxf_file->path, "r");


	while (line = fgets(line, MAX_LINE_LENGTH, txt_file)) {
		if (strcmp(line, "ENTITIES\r\n") == 0) {
			write = 1;
			line = fgets(line, MAX_LINE_LENGTH, txt_file);
			strcpy(text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
			line = fgets(line, MAX_LINE_LENGTH, txt_file);
			strcpy(text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
			is_new_figure(line, dxf_file, &n_types);
			line = fgets(line, MAX_LINE_LENGTH, txt_file); 
		}

		if (is_new_figure(line, dxf_file, &n_types)) {
			str_count[curr_prim] = curr_line;
			curr_prim++;
			curr_line = 0;
		}

		if (write) {
			strcpy(text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
		}

		if (strcmp(line, "ENDSEC\r\n") == 0 && write) {
			str_count[curr_prim] = curr_line;
			write = 0;
			break;
		}
	}
	fclose(txt_file);
}

static void find_control_dots(struct DxfFile *dxf_file)
{
	char *line;
	int count = 0, i = 0, n = 0, k = 0;
	line = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	dxf_file->primitives = (struct DxfPrimitive*)malloc(sizeof(struct DxfPrimitive) * dxf_file->n_primitives);
	for (i = 0; i < dxf_file->n_primitives; i++)
		dxf_file->primitives[i].points = (struct PointD*)malloc(sizeof(struct PointD) * str_count[i]);

	for (i = 0; i < dxf_file->n_primitives; i++) {
		n = 0;	
		for (k = 0; k < str_count[i]; k++) {
			strcpy(line, text_primitives[i].lines[k]);

			if (strcmp(line, " 10\r\n") == 0) {
				k++;
				count++;
				strcpy(line, text_primitives[i].lines[k]);
				dxf_file->primitives[i].points[n].x = atof(line);
			}
			if (strcmp(line, " 20\r\n") == 0) {
				k++;
				count++;
				strcpy(line, text_primitives[i].lines[k]);
				dxf_file->primitives[i].points[n].y = atof(line);
			}
			if (count == 2) {
				n++;
				count = 0;
			}
		}
		dxf_file->primitives[i].n_controldots = n;
	}
}




