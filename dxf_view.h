#include <math.h>

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
	*k = (p1.y - p2.y) / (p1.x - p2.x);
	*b = p1.y - (*k) * p2.x;
}

int cross_check(struct DxfFile curr_file, struct DxfFile pos_file, struct PointD offset, struct PointD pos_offset, int positioned)
{
	double xl1, xr1, yt1, yb1, xl2, xr2, yt2, yb2;
	double k1, k2, b1, b2;
	int i, j, k, m;

	xl1 = offset.x;
	xr1 = curr_file.x_max + offset.x;
	yb1 = offset.y;
	yt1 = curr_file.y_max + offset.y;

	xl2 = pos_offset.x;
	xr2 = pos_file.x_max + pos_offset.x;
	yb2 = pos_offset.y;
	yt2 = pos_file.y_max + pos_offset.y;

	if (xr1 < xl2)
		return 0;
	if (xl1 > xr2)
		return 0;
	if (yt1 < yb2)
		return 0;
	if (yb1 > yt2)
		return 0;
	else {
		xl1 = (xl1 > xl2)? xl1 : xl2;
		xr1 = (xr1 < xr2)? xr1 : xr2;
		yt1 = (yt1 < yt2)? yt1 : yt2;
		yb1 = (yb1 > yb2)? yb2 : yb1; 
	}


	for (i = 0; i < curr_file.n_primitives; i++) {
		for (j = 0; j < curr_file.n_controldots[i] - 1; j++) {
			struct PointD p_curr1, p_curr2;
			double k_curr, b_curr;
			double x_min_curr, x_max_curr;
			
			p_curr1 = curr_file.primitives[i].points[j];
			p_curr1.x += offset.x;
			p_curr1.y += offset.y;

			p_curr2 = curr_file.primitives[i].points[j + 1];
			p_curr2.x += offset.x;
			p_curr2.y += offset.y;

			if ((p_curr1.x <= xl1 && p_curr2.x <= xl1) || (p_curr1.x >= xr1 && p_curr2.x >= xr2))
				continue;
			if ((p_curr1.y <= yb1 && p_curr2.y <= yb1) || (p_curr1.y >= yt1 && p_curr2.y >= yt1))
				continue;
				
			determine_line(&k_curr, &b_curr, p_curr1, p_curr2);
			
			x_min_curr = (p_curr1.x < p_curr2.x)? p_curr1.x : p_curr2.x;
			x_max_curr = (p_curr1.x > p_curr2.x)? p_curr1.x : p_curr2.x;	
			for (k = 0; k < pos_file.n_primitives; k++) {
				for (m = 0; m < pos_file.n_controldots[k] - 1; m++) {
					struct PointD p_pos1, p_pos2;
					struct PointD cross_point;
					double k_pos, b_pos;
					double x_min_pos, x_max_pos;
					int res;

					p_pos1 = pos_file.primitives[k].points[m];
					p_pos1.x += pos_offset.x;
					p_pos1.y += pos_offset.y;
					
					p_pos2 = pos_file.primitives[k].points[m + 1];
					p_pos2.x += pos_offset.x;
					p_pos2.y += pos_offset.y;

					if ((p_pos1.x <= xl1 && p_pos2.x <= xl1) || (p_pos1.x >= xr1 && p_pos2.x >= xr1))
						continue;
					if ((p_pos1.y <= yb1 && p_pos2.y <= yb1) || (p_pos1.y >= yt1 && p_pos2.y >= yt1))
						continue;

					determine_line(&k_pos, &b_pos, p_pos1, p_pos2);

					res = find_line_cross(k_curr, b_curr, k_pos, b_pos, &cross_point);

					if (!res)
						continue;

					x_min_pos = (p_pos1.x < p_pos2.x)? p_pos1.x : p_pos2.x;
					x_max_pos = (p_pos1.x > p_pos2.x)? p_pos1.x : p_pos2.x;

					if ((cross_point.x >=  x_min_curr && cross_point.x >= x_min_pos) && (cross_point.x <= x_max_curr && cross_point.x <= x_max_pos))
							return 1;
				}
			}
		}
	}
	return 0;
}

void nest_dxf_stair(struct DxfFile *fdxf_files, int f_count, int width, int heigth)
{
	int i, j, k, m, can_place, extreme_rect_ind;
	int positioned, *how_many, max_position;
	double rect_xl, rect_yb, rect_xr, rect_yt;
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

	rect_xl = 0;
	rect_yb = 0;
	rect_xr = fdxf_files[0].x_max;
	rect_yt = fdxf_files[0].y_max;
	
	can_place = 1;
	while (can_place) {
			int placed = 0;
		for (i = 0; i < f_count;) {
			struct DxfFile curr_file;
			struct PointD offset, min_offset;
			struct Rectangle extreme_rect;
			int extreme_rect_ind;
			int was_placed;
			double min_square;
				
			if (how_many[i] == 0)
				i++;
			if (i == f_count)
					break;

			curr_file = fdxf_files[i];
			extreme_rect = curr_file.rects[0];
			for (j = 1; j < curr_file.n_rects; j++) {
				struct Rectangle tmp_rect;
				tmp_rect = curr_file.rects[j];	
				if (tmp_rect.x > extreme_rect.x) {
					extreme_rect = tmp_rect;
				}
			}
			
			min_square = -1;
			was_placed = 0;
			
			for (j = 0; j < positioned; j++) {
				struct DxfFile pos_file;
				pos_file = positions[j].file;				
				for (k = 0; k < pos_file.n_rects; k++) {
					struct PointD pos_offset;
					struct Rectangle pos_rect;
					double tmp_rect_xl, tmp_rect_xr, tmp_rect_yt, tmp_rect_yb;
					double tmp_square;
					int crossed;

					pos_rect = pos_file.rects[k];
					pos_offset.x = positions[j].x;
					pos_offset.y = positions[j].y;
					
					offset.x = (pos_rect.x + pos_offset.x) - extreme_rect.x;
					offset.y = (pos_rect.y + pos_offset.y) - extreme_rect.y;
					
					crossed = cross_check(curr_file, pos_file, offset, pos_offset, positioned);
					if (crossed)
							continue;

					tmp_rect_xl = (rect_xl < offset.x)? rect_xl : offset.x;
					tmp_rect_xr = (rect_xr > offset.x + curr_file.x_max)? rect_xr : offset.x + curr_file.x_max;
					tmp_rect_yb = (rect_yb < offset.y)? rect_yb : offset.y;
					tmp_rect_yt = (rect_yt > offset.y + curr_file.y_max)? rect_yt : offset.y + curr_file.y_max;
					
					tmp_square = (tmp_rect_xr - tmp_rect_xl) * (tmp_rect_yt - tmp_rect_yb);
					printf("tmp=%f min=%f\n", tmp_square, min_square);
					if (min_square == -1) {
						min_square = tmp_square;
						rect_xl = tmp_rect_xl;
						rect_xr = tmp_rect_xr;
						rect_yb = tmp_rect_yb;
						rect_yt = tmp_rect_yt;
						min_square = tmp_square;						
						min_offset = offset;
						was_placed = 1;
						printf("first_square\n");	
					} else if (tmp_square < min_square) {
						printf("here\n");
						rect_xl = tmp_rect_xl;
						rect_xr = tmp_rect_xr;
						rect_yb = tmp_rect_yb;
						rect_yt = tmp_rect_yt;	
						min_square = tmp_square;					
						min_offset = offset;		
					} else if (tmp_square == min_square) {
						if (fabs(offset.y) < fabs(min_offset.y)) {
							printf("here ==\n");
							rect_xl = tmp_rect_xl;
							rect_xr = tmp_rect_xr;
							rect_yb = tmp_rect_yb;
							rect_yt = tmp_rect_yt;	
							min_square = tmp_square;					
							min_offset = offset;	
						}
					}
				}
			}

			if (was_placed) {
					positions[positioned].file = curr_file;
					positions[positioned].x = min_offset.x;
					positions[positioned].y = min_offset.y;
					how_many[i] -= 1;
					
					if (how_many[i] == 0)
							i++;

					positioned++;
				
				if (positioned == max_position) {					
					max_position *= 2;
					positions = (struct Position*)realloc(positions, sizeof(struct Position) * max_position);
				}

				placed = 1;
			} else 
				i++;
		}

		if (!placed)
				break;
		
		can_place = 0;
		for (i = 0; i < f_count; i++) {
				if (how_many[i] > 0)
						can_place = 1;
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

	printf ("positioned = %d\n", positioned);	
	n_positioned = positioned;
	draw_nested(positions);
}

