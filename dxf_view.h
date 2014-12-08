#include <sys/types.h>
#include <pthread.h>

void show_dxf_file(struct DxfFile *dxf_file);
void nest_dxf(struct DxfFile *fdxf_files, int f_count, int width, int height);
void nest_dxf2(struct DxfFile *fdxf_files, int f_count, int width, int height);
int dxf_cmp(const void *file1, const void *file2); 

struct Position {
	struct DxfFile file;
	double x, y;
	int i;
};

struct Level {
	double from_x, to_x, y;
};

int n_positioned;

void draw_spline(struct DxfFile *dxf_file, cairo_t *cr, int i)
{
	int k;
	for (k = 0; k < dxf_file->n_controldots[i] - 2; k++) {
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

void draw_line(struct DxfFile *dxf_file, cairo_t *cr, int i)
{
	int k;

	printf("%d\n", dxf_file->n_controldots[i]);	
	for (k = 0; k < dxf_file->n_controldots[i] - 1; k++) {
		double x, y;
		x = dxf_file->primitives[i].points[k].x;
		y = dxf_file->primitives[i].points[k].y;	
		cairo_move_to(cr, x, y);
		x = dxf_file->primitives[i].points[k + 1].x;
		y = dxf_file->primitives[i].points[k + 1].y;
		cairo_line_to(cr, x, y);	
	}
}

gboolean on_draw_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	int i, k;
	struct DxfFile *dxf_file;

	dxf_file = (struct DxfFile*)user_data;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 1);		
	printf("%d\n", dxf_file->n_primitives);	
	for (i = 0; i < dxf_file->n_primitives; i++) {
		printf("%d\n", dxf_file->types[i]);	
		for (k = 0; k < dxf_file->n_controldots[i] - 1; k++) {
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

	cairo_set_source_rgb(cr, 0, 255, 0);
	cairo_rectangle(cr, 0, 0, dxf_file->x_max, dxf_file->y_max);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 255, 0, 0);
	for (i = 0; i < dxf_file->n_rects; i++) {
		double x, y, heigth, width;
		x = dxf_file->rects[i].x;
		y = dxf_file->rects[i].y;
		width = dxf_file->rects[i].width;
		heigth = dxf_file->rects[i].heigth;
		cairo_rectangle(cr, x, y, width, heigth);
	}


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

gboolean on_draw_nested_signal(GtkWidget *widget, cairo_t *cr, gpointer user_data)
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
		
		cairo_stroke(cr);

		cairo_set_source_rgb(cr, 255, 0, 0);
	/*	for (i = 0; i < dxf_file.n_rects; i++) {
			double x, y, heigth, width;
			x = dxf_file.rects[i].x + offset_x;
			y = dxf_file.rects[i].y + offset_y;
			width = dxf_file.rects[i].width;
			heigth = dxf_file.rects[i].heigth;
			cairo_rectangle(cr, x, y, width, heigth);
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


int dxf_cmp(const void *file1, const void *file2)
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

void nest_dxf2(struct DxfFile *fdxf_files, int f_count, int width, int heigth)
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

			
		double max_width, tmp_heigth, max_heigth;
		was_placed = 0;
		for (i = 0; i < f_count; i++) {
			curr_file = fdxf_files[i];
			level_width = levels[lmin_index].to_x - levels[lmin_index].from_x;
			tmp_heigth = levels[lmin_index].y + curr_file.m_heigth;
			if (!was_placed && curr_file.m_width <= level_width && tmp_heigth <= heigth && how_many[i] > 0) {
				max_index = i;
				max_index_pos = lmin_index;
				max_width = curr_file.m_width;
				was_placed = 1;
			} else if (curr_file.m_width <= level_width && curr_file.m_width > max_width && tmp_heigth <= heigth && how_many[i] > 0) {
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
				
			n_levels++;
			for (k = n_levels - 1; k > max_index_pos; k--) {
				levels[k] = levels[k - 1];
			}
			
			new_x = curr_file.m_width + levels[k].from_x;
			levels[max_index_pos].to_x = new_x;
			levels[max_index_pos].y += curr_file.m_heigth;
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

		printf("%d\n", can_place);
	}

	printf("positioned = %d\n", positioned);
	n_positioned = positioned;
	for (i = 0; i < positioned; i++) {
		printf("i=%d x=%f y=%f, nprim=%d  %s\n", positions[i].i, positions[i].x, positions[i].y, positions[i].file.n_primitives, positions[i].file.path);
	}
	
	draw_nested(positions);

}

void nest_dxf_stair(struct DxfFile *fdxf_files, int f_count, int width, int heigth)
{
	int i, j, k, m, can_place, extreme_rect_ind;
	int positioned, *how_many, max_position;
	struct Position *positions;	
	double x_minp, y_minp;

	how_many = (int*)malloc(sizeof(int) * f_count);

	for (i = 0; i < f_count; i++)
		how_many[i] = 1;

	qsort(fdxf_files, f_count, sizeof(struct DxfFile), dxf_cmp);
	
	max_position = f_count;
	positions = (struct Position*)malloc(sizeof(struct Position) * max_position);

	positions[0].file = fdxf_files[0];
	positions[0].x = 0;
	positions[0].y = 0;
	positions[0].i = 0;
	positioned = 1;

	how_many[0] -= 1;
	
	can_place = 1;
	while (can_place) {
		for (i = 0; i < f_count; i++) {
			struct DxfFile curr_file;
			struct PointD offset;
			struct Rectangle extreme_rect;
			int extreme_rect_ind;

			curr_file = fdxf_files[i];
			extreme_rect = curr_file.rects[0];
			for (j = 1; j < curr_file.n_rects; j++) {
				struct Rectangle tmp_rect;
				tmp_rect = curr_file.rects[j];	
				if (tmp_rect.x > extreme_rect.x) {
					extreme_rect = tmp_rect;
				}
			}

			for (j = 0; j < n_positioned; j++) {
				struct DxfFile pos_file;				
				for (k = 0; k < pos_file.n_rects; k++) {
					struct PointD pos_offset;
					struct Rectangle pos_rect;	
				}
			}
		}
	}
	
	x_minp = positions[0].x;
	y_minp = positions[0].y;

	for (k = 1; k < positioned; k++) {
		if (positions[k].x < x_minp)
			x_minp = positions[k].x;
		if (positions[k].y < y_minp)
			y_minp = positions[k].y;					
	}
	
	for (k = 0; k < positioned; k++) {
		positions[k].x -= x_minp;
		positions[k].y -= y_minp;
	}

		
	n_positioned = positioned;
	draw_nested(positions);
}

