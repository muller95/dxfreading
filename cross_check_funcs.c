#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "nestapi_core_structs.h"

int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset);


static double vec_prod(double x1, double y1, double x2, double y2)
{
    return x1 * y2 - x2 * y1;
}

static int cross_check_with_line(struct PointD lp1, struct PointD lp2, struct PointD vp1, struct PointD vp2)
{
    return vec_prod(lp2.x - lp1.x, lp2.y - lp1.y, vp1.x - lp1.x, vp1.y - lp1.y) * vec_prod(lp2.x - lp1.x, lp2.y - lp1.y, vp2.x - lp1.x, vp2.y - lp1.y) <= 0;
}

int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset)
{
	struct Polygon curr_poly, pos_poly;
	double xl1, xr1, yt1, yb1;
	double xl2, xr2, yt2, yb2;
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
		struct PointD cp1, cp2;
		cp1 = curr_poly.points[i];
		cp1.x += offset.x;
		cp1.y += offset.y;

		cp2 = curr_poly.points[i + 1];
		cp2.x += offset.x;
		cp2.y += offset.y;

        xl1 = (cp1.x < cp2.x)? cp1.x : cp2.x;
        xr1 = (cp1.x > cp2.x)? cp1.x : cp2.x;
        yb1 = (cp1.y < cp2.y)? cp1.y : cp2.y;
        yt1 = (cp1.y > cp2.y)? cp1.y : cp2.y;


		for (k = 0; k < pos_poly.n_points - 1; k++) {
			int res;
			struct PointD pp1, pp2, cross_point;
	
			pp1 = pos_poly.points[k];
			pp1.x += pos_offset.x;
			pp1.y += pos_offset.y;
			
			pp2 = pos_poly.points[k + 1];
			pp2.x += pos_offset.x;
			pp2.y += pos_offset.y;

            xl2 = (pp1.x < pp2.x)? pp1.x : pp2.x;
            xr2 = (pp1.x > pp2.x)? pp1.x : pp2.x;
            yb2 = (pp1.y < pp2.y)? pp1.y : pp2.y;
            yt2 = (pp1.y > pp2.y)? pp1.y : pp2.y;

            if (xl1 > xr2 || xr1 < xl2 || yb1 > yt2 || yt1 < yb2)
		        continue;
            
            if (!cross_check_with_line(pp1, pp2, cp1, cp2))
                continue;
            else if (!cross_check_with_line(cp1, cp2, pp1, pp2))
                continue;
            else 
                return 1;
		}
	}
	
	return 0;

}




