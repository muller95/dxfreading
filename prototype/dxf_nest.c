#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "nestapi_core_structs.h"

int n_positioned;

void show_dxf_file(struct DxfFile *dxf_file);
void nest_dxf(struct DxfFile *fdxf_files, int f_count, int width, int height);

static void draw_line(struct DxfFile *dxf_file, cairo_t *cr, int i);
static void draw_spline(struct DxfFile *dxf_file, cairo_t *cr, int i);
static int dxf_cmp(const void *file1, const void *file2); 
static gboolean on_draw_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_draw_nested_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static void draw_nested(struct Position *positions);
static int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point); 
static void determine_line(double *k, double *b, struct PointD p1, struct PointD p2);
static int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset, int positioned);

static void draw_spline(struct DxfFile *dxf_file, cairo_t *cr, int i)
{
	int k;
	for (k = 0; k < dxf_file->primitives[i].n_controldots - 2; k++) {
		double x1, y1, x2, y2, x3, y3;
		x1 = dxf_file->primitives[i].points[k].x;
		y1 = dxf_file->primitives[i].points[k].y;
		x2 = dxf_file->primitives[i].points[k + 1].x;
		y2 = dxf_file->primitives[i].points[k + 1].y;
		x3 = dxf_file->primitives[i].points[k + 2].x;
		y3 = dxf_file->primitives[i].points[k + 2].y;
		cairo_move_to(cr, x1, y1);
		cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);	
	}
}

static void draw_line(struct DxfFile *dxf_file, cairo_t *cr, int i)
{
	int k;

	for (k = 0; k < dxf_file->primitives[i].n_controldots - 1; k++) {
		double x, y;
		x = dxf_file->primitives[i].points[k].x;
		y = dxf_file->primitives[i].points[k].y;	
		cairo_move_to(cr, x, y);
		x = dxf_file->primitives[i].points[k + 1].x;
		y = dxf_file->primitives[i].points[k + 1].y;
		cairo_line_to(cr, x, y);	
	}
}

static gboolean on_draw_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	int i, k;
	struct DxfFile *dxf_file;

	dxf_file = (struct DxfFile*)user_data;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 1);		
	printf("%d\n", dxf_file->n_primitives);	
	for (i = 0; i < dxf_file->n_primitives; i++) {
		for (k = 0; k < dxf_file->primitives[i].n_controldots - 1; k++) {
			double x, y;
			x = dxf_file->primitives[i].points[k].x;
			y = dxf_file->primitives[i].points[k].y;	
			cairo_move_to(cr, x, y);
			x = dxf_file->primitives[i].points[k + 1].x;
			y = dxf_file->primitives[i].points[k + 1].y;
			cairo_line_to(cr, x, y);	
		}
	}
	cairo_stroke(cr);
	
	cairo_rectangle(cr, dxf_file->polygon.gravity_center.x, dxf_file->polygon.gravity_center.y, 3, 3);
	cairo_set_source_rgb(cr, 0, 255, 0);
	cairo_rectangle(cr, 0, 0, dxf_file->x_max, dxf_file->y_max);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 255, 0, 0);
/*	for (i = 0; i < dxf_file->n_rects; i++) {
		double x, y, height, width;
		x = dxf_file->rects[i].x;
		y = dxf_file->rects[i].y;
		width = dxf_file->rects[i].width;
		height = dxf_file->rects[i].height;
		cairo_rectangle(cr, x, y, width, height);
	}*/


	for (i = 0; i < dxf_file->polygon.n_points - 1; i++) {
		double x, y;
		x = dxf_file->polygon.points[i].x;
		y = dxf_file->polygon.points[i].y;
		cairo_move_to(cr, x, y);
		x = dxf_file->polygon.points[i + 1].x;
		y = dxf_file->polygon.points[i + 1].y;
		cairo_line_to(cr, x, y);
	}

	cairo_stroke(cr);
	return FALSE;	
}

static gboolean on_draw_nested_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	int i, k, j;
	struct Position *positions;

	positions = (struct Position*)user_data;

	cairo_set_line_width(cr, 1);		
	
	for (j = 0; j < n_positioned; j++) {
		struct DxfFile dxf_file;
		double offset_x, offset_y;
			
		dxf_file = positions[j].file;;
		offset_x = positions[j].x;
		offset_y = positions[j].y;
		printf("%s\n", dxf_file.path);	
		cairo_set_source_rgb(cr, 0, 0, 0);
		for (i = 0; i < dxf_file.n_primitives; i++) {
			for (k = 0; k < dxf_file.primitives[i].n_controldots - 1; k++) {
				double x, y;
				x = dxf_file.primitives[i].points[k].x + offset_x;
				y = dxf_file.primitives[i].points[k].y + offset_y;	
				cairo_move_to(cr, x, y);
				x = dxf_file.primitives[i].points[k + 1].x + offset_x;
				y = dxf_file.primitives[i].points[k + 1].y + offset_y;
				cairo_line_to(cr, x, y);	
			}
		}
		
		cairo_stroke(cr);

		cairo_set_source_rgb(cr, 255, 0, 0);
	/*	for (i = 0; i < dxf_file.n_rects; i++) {
			double x, y, height, width;
			x = dxf_file.rects[i].x + offset_x;
			y = dxf_file.rects[i].y + offset_y;
			width = dxf_file.rects[i].width;
			height = dxf_file.rects[i].height;
			cairo_rectangle(cr, x, y, width, height);
		}*/
		
		cairo_stroke(cr);
	}

	cairo_stroke(cr);	
	return FALSE;	
}


void show_dxf_file(struct DxfFile *dxf_file)
{
	GtkWidget *view_window, *draw_area;
	view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	draw_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(view_window), draw_area);

	g_signal_connect(G_OBJECT(draw_area), "draw", G_CALLBACK(on_draw_signal), dxf_file);

	gtk_window_set_default_size(GTK_WINDOW(view_window), 640, 480);	

	gtk_widget_show_all(view_window);


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


static int dxf_cmp(const void *file1, const void *file2)
{
	struct DxfFile *dxf_file1 = (struct DxfFile*)file1;	
	struct DxfFile *dxf_file2 = (struct DxfFile*)file2;
	
	if (dxf_file1->m_width < dxf_file2->m_width)
		return 1;
	else if (dxf_file1->m_width > dxf_file2->m_width)
		return -1;
	else
		return 0;
}

void nest_dxf(struct DxfFile *fdxf_files, int f_count, int width, int height)
{
	int *how_many, can_place, level_parts, min_width;
	int parts, curr_part, curr_level, n_levels;
	int positioned, max_position, was_placed;
	int i, k, j;
	struct DxfFile curr_file;
	struct Position *positions;
	struct Level *levels;

	how_many = (int*)malloc(sizeof(int) * f_count);

	for (i = 0; i < f_count; i++) {
		how_many[i] = 3;
	}

	qsort(fdxf_files, f_count, sizeof(struct DxfFile), dxf_cmp);
	
	can_place = 1;

	min_width = fdxf_files[f_count - 1].m_width;
	parts = width / min_width;
	
	if (width % min_width != 0)
		parts += 1;
	

	max_position = f_count;
	positions = (struct Position*)malloc(sizeof(struct Position) * max_position);
	levels = (struct Level*)malloc(sizeof(struct Level) * parts);
	
	
	levels[0].from_x = 0;
	levels[0].to_x = width - 1;
	levels[0].y = 0;	
	
	
	curr_part = 0;
	i = 0;
	curr_level = 0;
	n_levels = 1;
	positioned = 0;

	while (can_place) {
		double level_width;
		int max_index, max_index_pos, lmin_index;
		for (k = 0; k < n_levels; k++) {
			level_width = levels[k].to_x - levels[k].from_x;		
			if (level_width < min_width) {
				double y, y1;
				if (k == 0 && n_levels > 1)
					levels[k].y = levels[k + 1].y;
				else if (k > 0 && k < n_levels - 1) {
					y = levels[k - 1].y;
					y1 = levels[k + 1].y;
					
					if (y <= y1)
						levels[k].y = y;
					else if (y1 < y && y1 >= levels[j].y)
						levels[k].y = y1;
				} else if (k == n_levels - 1 && n_levels > 1)
					levels[k].y = levels[k - 1].y;
			}
		}

		for (k = 0; k < n_levels - 1; k++) {
			double y, y1;
			y = levels[k].y;
			y1 = levels[k + 1].y;
 				if (y == y1) {
			levels[k].to_x = levels[k + 1].to_x;
				for (j = k + 1; j < n_levels - 1; j++)
					levels[j] = levels[j + 1];
					n_levels--;
			}
		}
		
		lmin_index = 0;
		for (k = 1; k < n_levels; k++) {
			if (levels[k].y < levels[lmin_index].y) 
				lmin_index = k;
		}

			
		double max_width, tmp_height, max_height;
		was_placed = 0;
		for (i = 0; i < f_count; i++) {
			curr_file = fdxf_files[i];
			level_width = levels[lmin_index].to_x - levels[lmin_index].from_x;
			tmp_height = levels[lmin_index].y + curr_file.m_height;
			if (!was_placed && curr_file.m_width <= level_width && tmp_height <= height && how_many[i] > 0) {
				max_index = i;
				max_index_pos = lmin_index;
				max_width = curr_file.m_width;
				was_placed = 1;
			} else if (curr_file.m_width <= level_width && curr_file.m_width > max_width && tmp_height <= height && how_many[i] > 0) {
				max_width = curr_file.m_width;
				max_index = i;
				max_index_pos = lmin_index;
			}
		}

		if (was_placed) {
			double new_x;

			curr_file = fdxf_files[max_index];
			positions[positioned].i = max_index;
			positions[positioned].x = levels[max_index_pos].from_x;
			positions[positioned].y= levels[max_index_pos].y;
			positions[positioned].file = curr_file;
			positions[positioned].file.polygon = curr_file.polygon;
				
			n_levels++;
			for (k = n_levels - 1; k > max_index_pos; k--) {
				levels[k] = levels[k - 1];
			}
			
			new_x = curr_file.m_width + levels[k].from_x;
			levels[max_index_pos].to_x = new_x;
			levels[max_index_pos].y += curr_file.m_height;
			levels[max_index_pos + 1].from_x = new_x;

			positioned++;

			if (positioned == max_position) {					
				max_position *= 2;
				positions = (struct Position*)realloc(positions, sizeof(struct Position) * max_position);
			}

			how_many[max_index]--;
		}
		

		if (!was_placed) {
			double min_new_width, new_y;
			int min_index = -1;
			for (k = 0; k < n_levels - 1; k++) {
				double tmp_width = levels[k + 1].to_x - levels[k].from_x; 
				if (!min_index) {
					min_new_width = levels[k + 1].to_x - levels[k].from_x;
					min_index = i;
				} else if (tmp_width < min_new_width) {
					min_new_width = tmp_width;
					min_index = k;
				}
			}

			levels[min_index].to_x = levels[min_index].to_x;
			new_y = (levels[min_index].y > levels[min_index + 1].y)? levels[min_index].y : levels[min_index + 1].y;
			levels[min_index].y = new_y;

			for (k = min_index + 1; k < n_levels - 1; k++) {
				levels[k] = levels[k + 1];
			}

			n_levels--;
		}
		
		if (n_levels == 1) {
			can_place = 0;
			break;
		}

		can_place = 0;
		for (i = 0; i < f_count; i++) {
			if (how_many[i] > 0)
				can_place = 1; 
		}
	}

	n_positioned = positioned;	
	draw_nested(positions);
}
