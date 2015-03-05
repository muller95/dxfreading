#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "nestapi_core_structs.h"

void move_to_zero_int(struct DxfFileI *dxf_file);
void create_polygon_jarvis_int(struct DxfFileI *dxf_file);
void gravity_center_in_polygon_int(struct DxfFileI *dxf_file);
void rotate_polygon_int(struct DxfFileI *dxf_file, double angle);

static struct PointI get_start_point(struct DxfFileI *dxf_file);
static struct PointI get_vector(struct PointI point1, struct PointI point2);
static int vector_len(struct PointI vec);

void rotate_polygon_int(struct DxfFileI *dxf_file, double angle)
{
	int i, j, first;
	double angle_rads, x_max, y_max, x_min, y_min;
	struct PointI grav_p; 
	angle_rads = angle * M_PI / 180;

	grav_p = dxf_file->polygon.gravity_center;
	dxf_file->polygon.gravity_center.x = (int)trunc(grav_p.x * cos(angle_rads) - grav_p.y * sin(angle_rads) + 0.5);
    dxf_file->polygon.gravity_center.y = (int)trunc(grav_p.x * sin(angle_rads) + grav_p.y * cos(angle_rads) + 0.5); 

    
	for (i = 0; i < dxf_file->polygon.n_points; i++) {
		struct PointI p;
		p = dxf_file->polygon.points[i];
		dxf_file->polygon.points[i].x = (int)trunc(p.x * cos(angle_rads) - p.y * sin(angle_rads) + 0.5);
		dxf_file->polygon.points[i].y = (int)trunc(p.x * sin(angle_rads) + p.y * cos(angle_rads) + 0.5); 
	}
	
	x_max = dxf_file->polygon.points[0].x;
	x_min = dxf_file->polygon.points[0].x;
	y_max = dxf_file->polygon.points[0].y;
	y_min = dxf_file->polygon.points[0].y;	
			
	for (i = 1; i < dxf_file->polygon.n_points; i++) {
		int x_tmp, y_tmp;

		x_tmp = dxf_file->polygon.points[i].x;
		y_tmp = dxf_file->polygon.points[i].y;

		if (x_max < x_tmp)
			x_max = x_tmp;
		if (x_min > x_tmp)
			x_min = x_tmp;
		if (y_max < y_tmp)
			y_max = y_tmp;
		if (y_min > y_tmp)
			y_min = y_tmp;
	}

	for (i = 0; i < dxf_file->polygon.n_points; i++) {
		dxf_file->polygon.points[i].x -= x_min;
		dxf_file->polygon.points[i].y -= y_min;
	}

	dxf_file->x_max = x_max - x_min;
	dxf_file->y_max = y_max - y_min;

    dxf_file->polygon.gravity_center.x -= x_min;
    dxf_file->polygon.gravity_center.y -= y_min;

    dxf_file->x_min = 0;
    dxf_file->y_min = 0; 

//    gravity_center_in_polygon(dxf_file);
}

void move_to_zero_int(struct DxfFileI *dxf_file)
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
	dxf_file->m_height = dxf_file->y_max;	
}

static struct PointI get_start_point(struct DxfFileI *dxf_file)
{
	struct PointI p;
	int x, y;
	int x_min, y_min;
	
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

static struct PointI get_vector(struct PointI point1, struct PointI point2)
{
	struct PointI vector;
	vector.x = point2.x - point1.x;
	vector.y = point2.y - point1.y;
	return vector;
}

static int vector_len(struct PointI vec)
{	
	int len;
	len = (int)trunc(sqrt(pow(vec.x, 2) + pow(vec.y, 2)) + 0.5);
	return len;
}

void create_polygon_jarvis_int(struct DxfFileI *dxf_file)
{
	int n_points = 0;
	int n = 0;
	int i, j;
	int crossprod;
	struct PointI *points;
	struct PointI vector, temp_vector;
	struct PointI start_p, curr_p, temp_p, next_p;

	for (i = 0; i < dxf_file->n_primitives; i++) {
		n_points += dxf_file->n_controldots[i];
	}
	dxf_file->polygon.points = (struct PointI*)malloc(sizeof(struct PointI) * n_points);
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

void gravity_center_in_polygon_int(struct DxfFileI *dxf_file)
{
	int i, n, start, step, end, n_squares;
	int *squares;
	int square_sum, x_sum, y_sum;
	struct PointI *centers, tmp_p, tmp_p1;

	n = dxf_file->polygon.n_points;
	squares = (int*)malloc(sizeof(int) * n);
	centers = (struct PointI*)malloc(sizeof(struct PointI) * n - 1);

	tmp_p = dxf_file->polygon.points[0];
	tmp_p1 = dxf_file->polygon.points[1];

	start = (tmp_p.y <= tmp_p1.y)? 0 : n - 1;
	step = (tmp_p.y <= tmp_p1.y)? 1 : -1;
	end = (tmp_p.y <= tmp_p1.y)? n : 0;

	n_squares = 0;
	for (i = start; i != end; i += step) {
		struct PointI p1, p2, rect_gravity_center, triangle_gravity_center;
		int rect_square, triangle_square;
		int x_projection, y_min, y_max;
	
		p1 = dxf_file->polygon.points[i];
		p2 = dxf_file->polygon.points[i + step];
	    
        	
        if (p1.x == p2.x) {
            squares[n_squares] = 0;
            centers[n_squares].x = p1.x;
            centers[n_squares].y = (p1.y + p2.y) / 2;
            n_squares++;
            continue;
        } else if (p1.y == p2.y) {
            squares[n_squares] = (p2.x - p1.x) * p1.y;
            centers[n_squares].x = (p1.x + p2.x) / 2;
            centers[n_squares].y = p1.y / 2;
            n_squares++;
            continue;
        }
		
		x_projection = p2.x - p1.x;
		y_min = (p1.y < p2.y)? p1.y : p2.y;
		y_max = (p1.y > p2.y)? p1.y : p2.y;
		
		rect_square = x_projection * y_min;
		triangle_square = x_projection * (y_max - y_min) / 2;
		rect_gravity_center.x = x_projection / 2;
		rect_gravity_center.y = y_min / 2;
		
		triangle_gravity_center.y = (y_min + y_min + y_max) / 3;
		if (p1.y > p2.y)
			triangle_gravity_center.x = (p1.x + p1.x + p2.x) / 3;
		else 
			triangle_gravity_center.x = (p1.x + p2.x + p2.x) / 3; 
			

		squares[n_squares] = rect_square + triangle_square;
		centers[n_squares].x = (abs(rect_square) * rect_gravity_center.x + triangle_square * abs(triangle_gravity_center.x)) / (abs(rect_square + triangle_square));
		centers[n_squares].y = (abs(rect_square) * rect_gravity_center.y + triangle_square * abs(triangle_gravity_center.y)) / (abs(rect_square + triangle_square));
		n_squares++;
	}
	
	square_sum = x_sum = y_sum = 0.0;
	for (i = 0; i < n_squares; i++) {
		square_sum += squares[i];
		x_sum += centers[i].x * squares[i];
		y_sum += centers[i].y * squares[i];
	}

	dxf_file->polygon.gravity_center.x = x_sum / square_sum;
	dxf_file->polygon.gravity_center.y = y_sum / square_sum;
}

