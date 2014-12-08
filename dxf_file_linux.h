#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

#pragma once

#define MAX_LINE_LENGTH 2048

enum {
	DXF_TYPE_LINE,
	DXF_TYPE_SPLINE,
};

struct PointD {
	double x, y;
};

struct Rectangle {
	double x, y, width, heigth;
};

struct Polygon {
	struct PointD *points;
	int n_points;
};

struct DxfPrimitive {
	struct PointD *points;
	int type;
};

struct TextPrimitive {
	char **lines;
};

struct DxfFile {
	char *path;
	struct TextPrimitive *text_primitives;
	struct DxfPrimitive *primitives;
	struct Polygon polygon;
	struct Rectangle *rects;
	int n_primitives, n_types, n_rects; 
	int max_types, max_lines; 
	int *n_controldots, *str_count, *types;
	double m_heigth, m_width; 
	double x_min, x_max, y_min, y_max;
};

void dxf_file_new(struct DxfFile *dxf_file, char *path);
void read_entity(struct DxfFile *dxf_file);
void prepare_to_read(char *path, struct DxfFile *dxf_file);
void find_control_dots(struct DxfFile *dxf_file);
void move(struct DxfFile *dxf_file);
void create_polygon(struct DxfFile *dxf_file);
void create_polygon_jarvis(struct DxfFile *dxf_file);
double vector_len(struct PointD vec);
void aprox_rects(struct DxfFile *dxf_file);

struct PointD get_vector(struct PointD point1, struct PointD point2);
struct PointD get_start_point(struct DxfFile *dxf_file);

int is_new_figure(char *line, struct DxfFile *dxf_file, int curr_prim);
int is_new_figure2(char *line, struct DxfFile *dxf_file);

void dxf_file_new(struct DxfFile* dxf_file, char *path)
{
	int i;

	dxf_file->path = path;
	dxf_file->n_primitives = 0;
	dxf_file->n_types = 0;
	dxf_file->max_types = 0;
	dxf_file->max_lines = 0;	

	prepare_to_read(path, dxf_file);
	read_entity(dxf_file);
	find_control_dots(dxf_file);
	move(dxf_file);
	create_polygon_jarvis(dxf_file);
	aprox_rects(dxf_file);
}

int is_new_figure2(char *line, struct DxfFile *dxf_file)
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

int is_new_figure(char *line, struct DxfFile *dxf_file, int curr_prim)
{
	int n_type;

	if (strcmp(line, "LINE\r\n") == 0) {
		n_type = dxf_file->n_types;
		dxf_file->types[n_type] = DXF_TYPE_LINE;
		dxf_file->n_types++;
		return 1;
	} else if(strcmp(line, "SPLINE\r\n") == 0) {
		n_type = dxf_file->n_types;
		dxf_file->types[n_type] = DXF_TYPE_SPLINE;
		dxf_file->n_types++;
		return 1;
	}
	return 0;
}


void prepare_to_read(char *path, struct DxfFile *dxf_file)
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
	dxf_file->max_lines = curr_line;
}

void read_entity(struct DxfFile *dxf_file)
{
	int curr_line = 0, curr_prim = 0, write = 0, i = 0, j = 0;
	char *line;
	FILE *txt_file;

	line = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	dxf_file->primitives = (struct DxfPrimitive*)malloc(sizeof(struct DxfPrimitive) * dxf_file->n_primitives);
	dxf_file->str_count = (int*)malloc(sizeof(int) * dxf_file->n_primitives);
	dxf_file->text_primitives = (struct TextPrimitive*)malloc(sizeof(struct TextPrimitive) * dxf_file->n_primitives);
	dxf_file->types = (int*)malloc(sizeof(int) * dxf_file->n_primitives);	

	for (i = 0; i < dxf_file->n_primitives; i++) {
		dxf_file->text_primitives[i].lines = (char**)malloc(sizeof(char*) * dxf_file->max_lines);
		for (j = 0; j < dxf_file->max_lines; j++)
			dxf_file->text_primitives[i].lines[j] = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	}

	txt_file = fopen(dxf_file->path, "r");


	while (line = fgets(line, MAX_LINE_LENGTH, txt_file)) {
		if (strcmp(line, "ENTITIES\r\n") == 0) {
			write = 1;
			line = fgets(line, MAX_LINE_LENGTH, txt_file);
			strcpy(dxf_file->text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
			line = fgets(line, MAX_LINE_LENGTH, txt_file);
			strcpy(dxf_file->text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
			is_new_figure(line, dxf_file, curr_prim);
			line = fgets(line, MAX_LINE_LENGTH, txt_file); 
		}

		if (is_new_figure(line, dxf_file, curr_prim)) {
			dxf_file->str_count[curr_prim] = curr_line;
			curr_prim++;
			curr_line = 0;
		}

		if (write) {
			strcpy(dxf_file->text_primitives[curr_prim].lines[curr_line], line);
			curr_line++;
		}

		if (strcmp(line, "ENDSEC\r\n") == 0 && write) {
			dxf_file->str_count[curr_prim] = curr_line;
			write = 0;
			break;
		}
	}
	fclose(txt_file);
}

void find_control_dots(struct DxfFile *dxf_file)
{
	char *line;
	int count = 0, i = 0, n = 0, k = 0;
	line = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);
	dxf_file->n_controldots = (int*)malloc(sizeof(int) * dxf_file->n_primitives);
	dxf_file->primitives = (struct DxfPrimitive*)malloc(sizeof(struct DxfPrimitive) * dxf_file->n_primitives);
	for (i = 0; i < dxf_file->n_primitives; i++)
		dxf_file->primitives[i].points = (struct PointD*)malloc(sizeof(struct PointD) * dxf_file->str_count[i]);

	for (i = 0; i < dxf_file->n_primitives; i++) {
		n = 0;	
		for (k = 0; k < dxf_file->str_count[i]; k++) {
			strcpy(line, dxf_file->text_primitives[i].lines[k]);

			if (strcmp(line, " 10\r\n") == 0) {
				k++;
				count++;
				strcpy(line, dxf_file->text_primitives[i].lines[k]);
				dxf_file->primitives[i].points[n].x = atof(line);
			}
			if (strcmp(line, " 20\r\n") == 0) {
				k++;
				count++;
				strcpy(line, dxf_file->text_primitives[i].lines[k]);
				dxf_file->primitives[i].points[n].y = atof(line);
			}
			if (count == 2) {
				n++;
				count = 0;
			}
		}
		dxf_file->n_controldots[i] = n;
	}
}

void move(struct DxfFile *dxf_file)
{
	int first = 1;
	int i = 0, j = 0;

	for (j = 0; j < dxf_file->n_primitives; j++) {
		for (i = 0; i < dxf_file->n_controldots[j]; i++) {
			if (first) {
				dxf_file->x_min = dxf_file->primitives[j].points[i].x;
				dxf_file->y_min = dxf_file->primitives[j].points[i].y;
				dxf_file->x_max = dxf_file->primitives[j].points[i].x;
				dxf_file->y_max = dxf_file->primitives[j].points[i].y;
				first = 0;
			} else {
				if (dxf_file->primitives[j].points[i].x < dxf_file->x_min)
					dxf_file->x_min = dxf_file->primitives[j].points[i].x;
				if (dxf_file->primitives[j].points[i].y < dxf_file->y_min)
					dxf_file->y_min = dxf_file->primitives[j].points[i].y;
				if (dxf_file->primitives[j].points[i].x > dxf_file->x_max)
					dxf_file->x_max = dxf_file->primitives[j].points[i].x;
				if (dxf_file->primitives[j].points[i].y > dxf_file->y_max)
					dxf_file->y_max = dxf_file->primitives[j].points[i].y;
			}
		}
	}

	for (j = 0; j < dxf_file->n_primitives; j++) {
		for (i = 0; i < dxf_file->n_controldots[j]; i++) {
			dxf_file->primitives[j].points[i].x -= dxf_file->x_min;
			dxf_file->primitives[j].points[i].y -= dxf_file->y_min;
		}
	}

	dxf_file->x_max -= dxf_file->x_min;
	dxf_file->y_max -= dxf_file->y_min;
	dxf_file->m_width = dxf_file->x_max;
	dxf_file->m_heigth = dxf_file->y_max;
}

struct PointD get_start_point(struct DxfFile *dxf_file)
{
	struct PointD p;
	double x, y;
	double x_min, y_min;
	
	x_min = dxf_file->primitives[0].points[0].x;
	y_min = dxf_file->primitives[0].points[0].y;
	
	int i, j;
	for (i = 0; i < dxf_file->n_primitives; i++) {
		for (j = 0; j < dxf_file->n_controldots[i]; j++) {
			x = dxf_file->primitives[i].points[j].x;
			y = dxf_file->primitives[i].points[j].y;

			if (x_min > x) {
				p = dxf_file->primitives[i].points[j];
				x_min = x;
			}
			else if ((x_min == x) && (y_min > y)) {
				p = dxf_file->primitives[i].points[j];
				y_min = y;
			}
		}
	}
	return p;
}

struct PointD get_vector(struct PointD point1, struct PointD point2)
{
	struct PointD vector;
	vector.x = point2.x - point1.x;
	vector.y = point2.y - point1.y;
	return vector;
}

double vector_len(struct PointD vec)
{	
	double len;
	len = sqrt(pow(vec.x, 2) + pow(vec.y, 2));
	return len;
}

void create_polygon_jarvis(struct DxfFile *dxf_file)
{
	int n_points = 0;
	int n = 0;
	int i, j;
	double crossprod;
	struct PointD *points;
	struct PointD vector, temp_vector;
	struct PointD start_p, curr_p, temp_p, next_p;

	for (i = 0; i < dxf_file->n_primitives; i++) {
		n_points += dxf_file->n_controldots[i];
	}
	dxf_file->polygon.points = (struct PointD*)malloc(sizeof(struct PointD) * n_points);
	points = dxf_file->polygon.points;
		
	start_p = get_start_point(dxf_file);
	curr_p = start_p;
	temp_p = dxf_file->primitives[0].points[0];

	do {
		vector = get_vector(curr_p, temp_p);
		for (i = 0; i < dxf_file->n_primitives; i++) {
			for (j = 0; j < dxf_file->n_controldots[i]; j++) {
				temp_p = dxf_file->primitives[i].points[j];
				if (temp_p.x == curr_p.x && temp_p.y == curr_p.y)
					continue;
				temp_vector = get_vector(curr_p, temp_p);
				crossprod = vector.x * temp_vector.y - vector.y * temp_vector.x;
				if (crossprod > 0) {
				//	continue;
				}
				else if (crossprod < 0) {
					vector = temp_vector;
					next_p = temp_p;
				}
				else if (vector_len(temp_vector) > vector_len(vector)) {
					vector = temp_vector;
					next_p = temp_p;
				}
			}
		}
		points[n] = curr_p;
		n++;
		curr_p = next_p;
	} while ((start_p.x != curr_p.x || start_p.y != curr_p.y) && n < n_points-1);
	points[n] = start_p;
	n++;
	dxf_file->polygon.n_points = n;
}

int cmp(const void *a, const void *b)
{
	double y1, y2;

	y1 = *(double*)a;
	y2 = *(double*)b;

	if (y1 < y2)
		return -1;
	if (y1 > y2)
		return 1;
	else 
		return 0;	
}

void aprox_rects(struct DxfFile *dxf_file)
{
	int i, j, k, m, n, count;
	
	n = 0;
	for (i = 0; i < dxf_file->n_primitives; i++)
		n += dxf_file->n_controldots[i];

	
	count = 0;
		
	dxf_file->rects = (struct Rectangle*)malloc(sizeof(struct Rectangle) * (n - 1));
	
		for (i = 0; i < dxf_file->n_primitives; i++) {
		for (k = 0; k < dxf_file->n_controldots[i] - 1; k++) {
			double width, heigth, x, y, tmp, tmp1;

			tmp = dxf_file->primitives[i].points[k].x;
			tmp1 = dxf_file->primitives[i].points[k + 1].x;
			x = (tmp < tmp1)? tmp : tmp1;
			width = fabs(tmp - tmp1);

			tmp = dxf_file->primitives[i].points[k].y;
			tmp1 = dxf_file->primitives[i].points[k + 1].y;
			y = (tmp < tmp1)? tmp : tmp1;
			heigth = fabs(tmp - tmp1);
			dxf_file->rects[count].x = x;
			dxf_file->rects[count].y = y;
			dxf_file->rects[count].width = width;
			dxf_file->rects[count].heigth = heigth;
			count++;
		}

		dxf_file->n_rects = n - 1;
	}
}


