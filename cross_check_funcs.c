#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"

int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point);
int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point);
void determine_line(double *k, double *b, struct PointD p1, struct PointD p2);
int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset);

int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point) 
{
	cross_point->x = x;
	cross_point->y = k * x + b;
	return 1;
}

int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point) 
{
	double x, y;
	if (k1 == k2) 
		return 0;
	
	x = (b2 - b1) / (k1 - k2);
	y = k1 * x + b1;

	cross_point->x = x;
	cross_point->y = y;
	return 1;
}

void determine_line(double *k, double *b, struct PointD p1, struct PointD p2)
{
	double m_k, m_b;
	m_k = (p1.y - p2.y) / (p1.x - p2.x);
	m_b = p1.y - (m_k) * p1.x;
	*k = m_k;
	*b = m_b;
}

int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset)
{
	struct Polygon curr_poly, pos_poly;
	double xl1, xr1, yt1, yb1;
	double xl2, xr2, yt2, yb2;
	double k1, k2, b1, b2;
	int i, k;

	xl1 = offset.x;
	xr1 = curr_file.x_max + offset.x;
	yb1 = offset.y;
	yt1 = curr_file.y_max + offset.y;

	xl2 = pos_offset.x;
	xr2 = pos_file.x_max + pos_offset.x;
	yb2 = pos_offset.y;
	yt2 = pos_file.y_max + pos_offset.y;

	if (xl1 >= xr2 || xr1 <= xl2 || yb1 >= yt2 || yt1 <= yb2)
		return 0;
			
	curr_poly = curr_file.polygon;
	pos_poly = pos_file.polygon;

	for (i = 0; i < curr_poly.n_points - 1; i++) {
		struct PointD c_p1, c_p2;
		c_p1 = curr_poly.points[i];
		c_p1.x += offset.x;
		c_p1.y += offset.y;

		c_p2 = curr_poly.points[i + 1];
		c_p2.x += offset.x;
		c_p2.y += offset.y;

		determine_line(&k1, &b1, c_p1, c_p2);
		for (k = 0; k < pos_poly.n_points - 1; k++) {
			int res;
			struct PointD p_p1, p_p2, cross_point;
	
			p_p1 = pos_poly.points[k];
			p_p1.x += pos_offset.x;
			p_p1.y += pos_offset.y;
			
			p_p2 = pos_poly.points[k + 1];
			p_p2.x += pos_offset.x;
			p_p2.y += pos_offset.y;
			
			determine_line(&k2, &b2, p_p1, p_p2);
			if (fabs(k1) != INFINITY && fabs(k2) != INFINITY)
				res = find_line_cross(k1, b1, k2, b2, &cross_point);
			if (fabs(k1) != INFINITY && fabs(k2) == INFINITY)
				res = find_line_cross_inf(k1, b1, p_p1.x, &cross_point);
			if (fabs(k1) == INFINITY && fabs(k2) != INFINITY)
				res = find_line_cross_inf(k2, b2, c_p1.x, &cross_point);
			if (fabs(k1) == INFINITY && fabs(k2) == INFINITY) 
				continue;
	

			if (!res)
				continue;
			
			xl1 = (c_p1.x < c_p2.x)? c_p1.x : c_p2.x;
			xr1 = (c_p1.x > c_p2.x)? c_p1.x : c_p2.x;
			yt1 = (c_p1.y > c_p2.y)? c_p1.y : c_p2.y;
			yb1 = (c_p1.y < c_p2.y)? c_p1.y : c_p2.y;

			xl2 = (p_p1.x < p_p2.x)? p_p1.x : p_p2.x;
			xr2 = (p_p1.x > p_p2.x)? p_p1.x : p_p2.x;
			yt2 = (p_p1.y > p_p2.y)? p_p1.y : p_p2.y;
			yb2 = (p_p1.y < p_p2.y)? p_p1.y : p_p2.y;



			if ((cross_point.x > xl1 && cross_point.x < xr1) && (cross_point.x > xl2 && cross_point.x < xr2) && (cross_point.y > yb1 && cross_point.y < yt1) && (cross_point.y > yb2 && cross_point.y < yt2)) 
				return 1;
		}
	}
	
	return 0;
}




