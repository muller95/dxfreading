#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"

int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point);
int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point);
void determine_line(double *k, double *b, struct PointD p1, struct PointD p2);
int cross_check_nfp(struct DxfFile curr_file, struct PointD offset, struct NfpPoint *head);
int out_of_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, double width);

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

int cross_check_nfp(struct DxfFile curr_file, struct PointD offset, struct NfpPoint *head)
{
	struct Polygon curr_poly;
    double k1, b1, k2, b2;
    int i, k;
			
	curr_poly = curr_file.polygon;

	for (i = 0; i < curr_poly.n_points - 1; i++) {
		struct PointD cp1, cp2;
        struct NfpPoint *curr, *next;

		cp1 = curr_poly.points[i];
		cp1.x += offset.x;
		cp1.y += offset.y;

		cp2 = curr_poly.points[i + 1];
		cp2.x += offset.x;
		cp2.y += offset.y;

        curr = head;
        next = curr->next;

		determine_line(&k1, &b1, cp1, cp2);
		while(curr->next != NULL) {
			int res;
            double xl1, xr1, yt1, yb1, xl2, xr2, yt2, yb2;
			struct PointD pp1, pp2, cross_point;
	
			pp1 = curr->point;	
			pp2 = next->point;
      /*      if (n_positioned > 0) {
                printf("pp1 x=%f y=%f\n", pp1.x, pp1.y);
                printf("pp2 x=%f y=%f\n", pp2.y, pp2);
                getchar();
            }*/

            curr = next;
            next = next->next;
	
			determine_line(&k2, &b2, pp1, pp2);
			if (fabs(k1) != INFINITY && fabs(k2) != INFINITY)
				res = find_line_cross(k1, b1, k2, b2, &cross_point);
			if (fabs(k1) != INFINITY && fabs(k2) == INFINITY)
				res = find_line_cross_inf(k1, b1, pp1.x, &cross_point);
			if (fabs(k1) == INFINITY && fabs(k2) != INFINITY)
				res = find_line_cross_inf(k2, b2, cp1.x, &cross_point);
			if (fabs(k1) == INFINITY && fabs(k2) == INFINITY) 
				continue;
	

			if (!res)
				continue;
			
			xl1 = (cp1.x < cp2.x)? cp1.x : cp2.x;
			xr1 = (cp1.x > cp2.x)? cp1.x : cp2.x;
			yt1 = (cp1.y > cp2.y)? cp1.y : cp2.y;
			yb1 = (cp1.y < cp2.y)? cp1.y : cp2.y;

			xl2 = (pp1.x < pp2.x)? pp1.x : pp2.x;
			xr2 = (pp1.x > pp2.x)? pp1.x : pp2.x;
			yt2 = (pp1.y > pp2.y)? pp1.y : pp2.y;
			yb2 = (pp1.y < pp2.y)? pp1.y : pp2.y;



			if ((cross_point.x > xl1 && cross_point.x < xr1) && (cross_point.x >= xl2 && cross_point.x <= xr2) && 
                (cross_point.y >= yb1 && cross_point.y <= yt1) && (cross_point.y >= yb2 && cross_point.y <= yt2)) 
				return 1;
		}
	}
	
	return 0;
}

int out_of_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, double width)
{
    int i;

    for (i = 0; i < curr_file.polygon.n_points; i++) {
        double sum, eps = 0.01, k1, b1;
        int c;
        struct NfpPoint *curr, *next;
        struct PointD tp1, tp2;

        tp1 = curr_file.polygon.points[i];
        tp1.x += offset.x;
        tp1.y += offset.y;
        tp2.x = width * 2;
        tp2.y = tp1.y;

        curr = head;
        next = curr->next;
        
        k1 = 0;
        b1 = tp1.x;
        c = 0;
        while (curr->next != NULL) {
             double k2, b2;
             struct PointD np1, np2, cross_point;
             double xl1, xr1, xl2, xr2, yt2, yb2;
             int res;
             np1 = curr->point;
             np2 = next->point;

             curr = next;
             next = next->next;

             if ((tp1.x == np1.x && tp1.y == np1.y) || (tp1.x == np2.x && tp1.y == np2.y))
                 continue;

             determine_line(&k2, &b2, np1, np2);
             
             if (fabs(k2) == INFINITY)
				res = find_line_cross_inf(k1, b1, np1.x, &cross_point);
             else
                res = find_line_cross(k1, b1, k2, b2, &cross_point);

             if (!res)
                 continue;
            
            xl1 = tp1.x;
			xr1 = tp2.x;

			xl2 = (np1.x < np2.x)? np1.x : np2.x;
			xr2 = (np1.x > np2.x)? np1.x : np2.x;
			yt2 = (np1.y > np2.y)? np1.y : np2.y;
			yb2 = (np1.y < np2.y)? np1.y : np2.y;
                
            if ((cross_point.x > xl1 && cross_point.x < xr1) && (cross_point.x >= xl2 && cross_point.x <= xr2) && (cross_point.y > yb2 && cross_point.y < yt2))  
                c += 1;           
        }
      //  printf("c=%d \n",c);
       // getchar();
        if (c % 2 == 0 && c > 0)
            return 1;
    }

    return 0;
}
