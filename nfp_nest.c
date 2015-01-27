#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"

struct Generation {
    int *genom;
    int genon_size;
};

int n_positioned;
int max_generations, n_generations;
double min_height;
struct Genration *generations;

void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double width, double height);

static void generate_first_generation(struct DxfFile *dxf_files, int f_count, double width, double height);
static void rotate_polygon(struct DxfFile *dxf_file, double angle);
static int dxf_cmp(const void *file1, const void *file2);
static int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point);
static int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point);
static void determine_line(double *k, double *b, struct PointD p1, struct PointD p2);
static int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset);

static gboolean on_draw_nested_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	int i, k, j;
	struct Position *positions;

	positions = (struct Position*)user_data;

	cairo_set_line_width(cr, 1);		
	
/*	for (j = 0; j < n_positioned; j++) {
		struct DxfFile dxf_file;
		double offset_x, offset_y;
				
		dxf_file = positions[j].file;
		offset_x = positions[j].x;
		offset_y = positions[j].y;
		printf("%s\n", dxf_file.path);	
		cairo_set_source_rgb(cr, 0, 0, 0);
		for (i = 0; i < dxf_file.n_primitives; i++) {
			for (k = 0; k < dxf_file.n_controldots[i] - 1; k++) {
				double x, y;
				x = dxf_file.primitives[i].points[k].x + offset_x;
				y = dxf_file.primitives[i].points[k].y + offset_y;	
				cairo_move_to(cr, x, y);
				x = dxf_file.primitives[i].points[k + 1].x + offset_x;
				y = dxf_file.primitives[i].points[k + 1].y + offset_y;
				cairo_line_to(cr, x, y);	
		}
	}
	
	cairo_stroke(cr); */

	cairo_set_source_rgb(cr, 255, 0, 0);
	for (j = 0; j < n_positioned; j++) {
			double offset_x, offset_y, g_x, g_y;
			struct DxfFile dxf_file;

			dxf_file = positions[j].file;
			offset_x = positions[j].x;
			offset_y = positions[j].y;
		for (i = 0; i < dxf_file.polygon.n_points - 1; i++) {
			double x, y;
			x = dxf_file.polygon.points[i].x;
			y = dxf_file.polygon.points[i].y;
			cairo_move_to(cr, x + offset_x, y + offset_y);
			
			x = dxf_file.polygon.points[i + 1].x;
			y = dxf_file.polygon.points[i + 1].y;
			cairo_line_to(cr, x + offset_x, y + offset_y);
		}		
			
		g_x = dxf_file.polygon.gravity_center.x;
		g_y = dxf_file.polygon.gravity_center.y;

		cairo_rectangle(cr, g_x + offset_x, g_y + offset_y, 3, 3);

		cairo_stroke(cr);
	}

	cairo_set_source_rgb(cr, 0, 255, 0);
	for (j = 0; j < n_positioned; j++) {
			double offset_x, offset_y, g_x, g_y;
			struct DxfFile dxf_file;

			dxf_file = positions[j].file;
			offset_x = positions[j].x;
			offset_y = positions[j].y;
		for (i = 0; i < dxf_file.polygon.n_points; i++) {
			double x, y;
			x = dxf_file.polygon.points[i].x;
			y = dxf_file.polygon.points[i].y;
			cairo_rectangle(cr, x + offset_x, y + offset_y, 3, 3);
		}		
			
		g_x = dxf_file.polygon.gravity_center.x;
		g_y = dxf_file.polygon.gravity_center.y;

		cairo_rectangle(cr, g_x + offset_x, g_y + offset_y, 3, 3);

		cairo_stroke(cr);
	}




	cairo_stroke(cr);	
	return FALSE;	
}

void draw_nested(struct Position *positions)
{

	GtkWidget *view_window, *draw_area;
	view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	draw_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(view_window), draw_area);

	g_signal_connect(G_OBJECT(draw_area), "draw", G_CALLBACK(on_draw_nested_signal), positions);

	gtk_window_set_default_size(GTK_WINDOW(view_window), 640, 480);	

	gtk_widget_show_all(view_window);

}

static void rotate_polygon(struct DxfFile *dxf_file, double angle)
{
	int i, j, first;
	double angle_rads, x_max, y_max, x_min, y_min;
	struct PointD ref_p; 
	angle_rads = angle * M_PI / 180;
	ref_p = dxf_file->polygon.gravity_center;

	for (i = 0; i < dxf_file->polygon.n_points; i++) {
		struct PointD p;

		ref_p = dxf_file->polygon.gravity_center;
		p = dxf_file->polygon.points[i];
		dxf_file->polygon.points[i].x = ref_p.x + (p.x - ref_p.x) * cos(angle_rads) - (p.y - ref_p.y) * sin(angle_rads);
		dxf_file->polygon.points[i].y = ref_p.y + (p.x - ref_p.x) * sin(angle_rads) + (p.y - ref_p.y) * cos(angle_rads); 
	}
	
	x_max = dxf_file->polygon.points[0].x;
	x_min = dxf_file->polygon.points[0].x;
	y_max = dxf_file->polygon.points[0].y;
	y_min = dxf_file->polygon.points[0].y;				
	for (i = 1; i < dxf_file->polygon.n_points; i++) {
		double x_tmp, y_tmp;

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
}

static int dxf_cmp(const void *file1, const void *file2)
{
	double square1, square2;
	struct DxfFile *dxf_file1, *dxf_file2;

	dxf_file1 = (struct DxfFile*)file1;	
	dxf_file2 = (struct DxfFile*)file2;

	square1 = dxf_file1->m_width * dxf_file1->m_height;
	square2 = dxf_file2->m_width * dxf_file2->m_height;
	
	if (square1 < square2)
		return 1;
	else if (square1 > square2)
		return -1;
	else
		return 0;
}

static int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point) 
{
	cross_point->x = x;
	cross_point->y = k * x + b;
	return 1;
}

static int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point) 
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

static void determine_line(double *k, double *b, struct PointD p1, struct PointD p2)
{
	double m_k, m_b;
	m_k = (p1.y - p2.y) / (p1.x - p2.x);
	m_b = p1.y - (m_k) * p1.x;
	*k = m_k;
	*b = m_b;
}

static int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset)
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




static void generate_first_generation(struct DxfFile *dxf_files, int f_count, double width, double height)
{
	int i, j, k, m; 
	int can_place, max_position, positioned;
	int *how_many;
	double curr_height;
	struct Position *positions;
	
	max_position = f_count;
	positions = (struct Position*)malloc(sizeof(struct Position) * max_position);
	how_many = (int*)malloc(sizeof(int) * f_count);	
	qsort(dxf_files, f_count, sizeof(struct DxfFile), dxf_cmp);
	
    curr_height = 0;
	for (i = 0; i < f_count; i++) 
		how_many[i] = 4;

	positioned = 0;
	can_place = 1;

	for (i = 0; i < f_count;) {
		int angle, angle_step;
		int res, was_placed;
		double min_length, x, y;
		struct DxfFile curr_file;
					
		curr_file = filedup(dxf_files[i]);
		angle_step = 15;
		min_length = -1;
	    
        was_placed = 0;
		for (x = 0.0; x < width - curr_file.m_width ; x += 1.0) {
			res = 0;
			for (y = height + curr_file.m_height; y >= 0; y -= 1.0) {
				double g_x, g_y, tmp_length;
				for (j = 0; j < positioned; j++) {
					int pos_ind;
					struct DxfFile pos_file;
					struct PointD offset, pos_offset;

					pos_file = positions[j].file;
					
					offset.x = x;
					offset.y = y;			
					pos_offset.x = positions[j].x;
					pos_offset.y = positions[j].y;

					res = cross_check(curr_file, pos_file, offset, pos_offset);
					if (res == 1)  
						break;
				}
				if (res == 1) 
					break;
						
				if (curr_file.y_max + y > height)				
					continue;
		
				g_x = curr_file.polygon.gravity_center.x + x;
				g_y = curr_file.polygon.gravity_center.y + y;
				tmp_length  = sqrt(pow(g_x, 2) + pow(g_y, 2));			

				if (min_length == -1 || tmp_length < min_length) {
                    was_placed = 1;
					min_length = tmp_length;
					positions[positioned].file = curr_file;
					positions[positioned].x = x;
					positions[positioned].y = y;
				} else if (tmp_length == min_length && positions[positioned].y > y) {
					positions[positioned].file = curr_file;
					positions[positioned].x = x;
					positions[positioned].y = y;
				}
                
                if (y == 0) {
                    x = width;
                    break;
                }
			}
		}

        if (!was_placed) {
            i++;
            continue;
        }

        curr_height = (curr_height > positions[positioned].y + curr_file.y_max)? curr_height : positions[positioned].y + curr_file.y_max;

		positioned++;
		printf("positioned = %d n offset.x=%f offset.y=%f height=%f %s\n", positioned, positions[positioned-1].x,
				positions[positioned-1].y, positions[positioned - 1].file.m_height, positions[positioned - 1].file.path);
		if (positioned == max_position) {					
			max_position *= 2;
			positions = (struct Position*)realloc(positions, sizeof(struct Position) * max_position);
		}

		how_many[i]--;
		if (how_many[i] == 0)
			i++;
	}

    min_height = (min_height < curr_height)? min_height : curr_height;

	
	printf("positioned = %d\n", positioned);
	n_positioned = positioned;
	draw_nested(positions);	
}


void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double width, double height)
{
    int i;
    struct DxfFile *tmp_dxf_files;

    min_height = height;
    max_generations = f_count;
    n_generations = 0;

//    generations = (struct Generation*)malloc(sizeof(struct Generation) * max_generations);
    tmp_dxf_files = (struct DxfFile*)malloc(sizeof(struct DxfFile) * f_count);
    
    for (i =0; i < f_count; i++)
        tmp_dxf_files[i] = filedup(dxf_files[i]);

	generate_first_generation(tmp_dxf_files, f_count, width, height);
}
