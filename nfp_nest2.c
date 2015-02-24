#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gtk/gtk.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"
#include "dxf_geometry.h"
#include "cross_check_funcs.h"

static int n_positioned, was_placed;
static int max_individs, n_individs, max_genom;
static double min_height, width, height;
static int **mask;

struct Individ *individs;

void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double w, double h);

static void recursive_move_x(double *x_pos, double *y_pos, double *min_length, struct DxfFile *curr_file, struct Position *positions, int positioned);
static int crossover();
static int individs_equal(struct Individ individ1, struct Individ individ2);
static int individs_cmp(const void *individ1, const void *individ2);
static void calculate_fitness();
static int calculate_individ_height(struct Individ individ, struct DxfFile *dataset, int draw);
static struct DxfFile* generate_dataset(struct DxfFile *dxf_files, int f_count, int *dataset_size);
static void mutate_individ(int n, int genom_size);
static int left_hole_nesting(struct DxfFile *dataset, struct Position *positions, int *max_position, int positioned, int dataset_size);
static void generate_first_individ(struct DxfFile *dataset, int dataset_size);
static int dxf_cmp(const void *file1, const void *file2);

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
    cairo_rectangle(cr, 0, 0, width, height);
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

static int left_hole_nesting(struct DxfFile *dataset, struct Position *positions, int *max_position, int positioned, int dataset_size)
{
    int i, j, genom_size;
    for (i = 0; i < dataset_size; i++) {
        int res;
        double y;
        struct DxfFile curr_file;
        
        curr_file = filedup(dataset[i]);
        was_placed = 0;
        for (y = -1 * curr_file.m_height; y <= height - curr_file.m_height; y++) {
        	double g_x, g_y;
			for (j = 0; j < positioned; j++) {
				int pos_ind;
				struct DxfFile pos_file;
				struct PointD offset, pos_offset;
                
				pos_file = positions[j].file;
					
				offset.x = 0;
				offset.y = y;			
				pos_offset.x = positions[j].x;
				pos_offset.y = positions[j].y;

				res = cross_check(curr_file, pos_file, offset, pos_offset);
				if (res == 1)  
					break;
			}
			if (res == 1) 
				break;
						
			if (y < 0)				
				continue;
		
			g_x = curr_file.polygon.gravity_center.x + 0;
			g_y = curr_file.polygon.gravity_center.y + y;		

            was_placed = 1;
			positions[positioned].file = curr_file;
			positions[positioned].x = 0;
			positions[positioned].y = y;             
        }

        if (!was_placed)
            continue;

        genom_size = individs[0].genom_size;
        individs[0].genom[genom_size] = i;
        individs[0].genom_size += 1;

        positioned++;
		printf("hole positioned = %d %s\n", positioned, positions[positioned - 1].file.path);
		if (positioned == *max_position) {					
			*max_position *= 2;
			positions = (struct Position*)realloc(positions, sizeof(struct Position) * (*max_position));
		}
    }

    return positioned;
}

static void init_mask(int w, int h)
{
    int i;

    mask = (int**)malloc(sizeof(int*) * (h * 2));

    for (i = 0; i < h * 2; i++) 
        mask[i] = (int*)calloc(w * 2, sizeof(int));
}

static void free_mask(int **mask, int w, int h)
{
    int i;

    for (i = 0; i < height * 2; i++)
        free(mask[i]);

    free(mask);
}

static void recursive_move_x(double *x_pos, double *y_pos, double *min_length, struct DxfFile *curr_file, struct Position *positions, int positioned)
{
    int j;
    double x, y;
    y = *y_pos;

    for (x = *x_pos; x >= 0; x -= 1.0) {
        int res;
        double g_x, g_y, tmp_length;
 //       mask[(int)x][(int)y] = 1;
        for (j = 0; j < positioned; j++) {
            int pos_ind;
	    	struct DxfFile pos_file;
		    struct PointD offset, pos_offset;

			pos_file = positions[j].file;
				    	
			offset.x = x;
			offset.y = y;			
			pos_offset.x = positions[j].x;
			pos_offset.y = positions[j].y;

			res = cross_check(*curr_file, pos_file, offset, pos_offset);
			if (res == 1)  
			   	break;
        }

        if (res == 1) 
		 	break;

        *x_pos = x;

		if (curr_file->y_max + y > height)				
			continue;

		g_x = curr_file->polygon.gravity_center.x + *x_pos;
		g_y = curr_file->polygon.gravity_center.y + *y_pos;
		tmp_length  = sqrt(pow(g_x, 2) + pow(g_y, 2));
                			
        if (*x_pos + curr_file->x_max > width)
            continue;

		if (*min_length == -1 || tmp_length < *min_length) {
            was_placed = 1;
    		*min_length = tmp_length;
    		positions[positioned].file = *curr_file;
			positions[positioned].x = *x_pos;
			positions[positioned].y = y;
		} else if (tmp_length == *min_length && positions[positioned].y > y) {
			positions[positioned].file = *curr_file;
			positions[positioned].x = *x_pos;
			positions[positioned].y = y;
		}
    }           
}

static void generate_first_individ(struct DxfFile *dataset, int dataset_size)
{
	int i, j, k, m, genom_size; 
	int max_position, positioned;
	double curr_height;
	struct Position *positions;
	
	max_position = dataset_size;
	positions = (struct Position*)malloc(sizeof(struct Position) * max_position);
		
    curr_height = 0;

    individs[0].genom = (int*)malloc(sizeof(int) * max_genom);
    individs[0].genom_size = 0;

	positioned = 0;

	for (i = 0; i < dataset_size; i++) {
		int res;
		double min_length, x, y;
		struct DxfFile curr_file;
					
		curr_file = filedup(dataset[i]);
		min_length = -1;
	    
        was_placed = 0;
       // init_mask((int)width, (int)height);
		for (x = 0.0; x <= width; x += 1.0) {
			res = 0;
			for (y = trunc(curr_height + 1.0); y >= 0; y -= 1.0) {
				double g_x, g_y, tmp_length, x_pos, y_pos;
                int xi, yi;
                
         //       xi = (int)x;
          //      yi = (int)y;
               // printf("xi=%d yi=%d c=%d\n", xi, yi, mask[0][0]);
            //    mask[xi][yi] = 1;
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
                
                x_pos = x;
                y_pos = y;

               // if (x_pos - 1.0 >= 0 && mask[(int)(x_pos - 1.0)][(int)(y_pos - 1.0)] != 1)
                recursive_move_x(&x_pos, &y_pos, &min_length, &curr_file, positions, positioned);
                    	
				if (curr_file.y_max + y > height)				
					continue;

				g_x = curr_file.polygon.gravity_center.x + x_pos;
				g_y = curr_file.polygon.gravity_center.y + y;
				tmp_length  = sqrt(pow(g_x, 2) + pow(g_y, 2));
                			
                if (x_pos + curr_file.x_max > width)
                    continue;

				if (min_length == -1 || tmp_length < min_length) {
                    was_placed = 1;
					min_length = tmp_length;
					positions[positioned].file = curr_file;
					positions[positioned].x = x_pos;
					positions[positioned].y = y;
				} else if (tmp_length == min_length && positions[positioned].y > y) {
					positions[positioned].file = curr_file;
					positions[positioned].x = x_pos;
					positions[positioned].y = y;
				}
                
                if (y == 0) {
                    x = width * 2;
                    break;
                }
			}
		}
        
     //   free_mask(mask, (int)width, (int)height);

        if (!was_placed) 
            continue;

		positioned++;

        if (positioned == 1)
            positioned = left_hole_nesting(dataset, positions, &max_position, positioned, dataset_size);

        genom_size = individs[0].genom_size;
        individs[0].genom[genom_size] = i;
        individs[0].genom_size += 1;
            
		printf("positioned = %d n %s\n", positioned, positions[positioned - 1].file.path);
		if (positioned == max_position) {					
			max_position *= 2;
			positions = (struct Position*)realloc(positions, sizeof(struct Position) * max_position);
		}

        curr_height = (curr_height > positions[positioned - 1].y + curr_file.y_max)? curr_height : positions[positioned - 1].y + curr_file.y_max;
	}

    min_height = (min_height < curr_height)? min_height : curr_height;
    individs[0].height = curr_height;

    n_individs = 1;
	
	printf("positioned = %d\n", positioned);
	n_positioned = positioned;

    draw_nested(positions);
}

static void mutate_individ(int n, int genom_size)
{
    int n1, n2, c, i;
    
    srand(time(NULL));

    n1 = (int)rand() % (genom_size);
    n2 = (int)rand() % (genom_size);

    while (n1 == n2) 
        n2 = (int)rand() % (genom_size);
    
    individs[n_individs].genom = (int*)malloc(sizeof(int) * max_genom);
    individs[n_individs].genom_size = individs[n].genom_size;
    individs[n_individs].height = -1;

    for (i = 0; i < individs[n].genom_size; i++)
        individs[n_individs].genom[i] = individs[n].genom[i];

    c = individs[n].genom[n1];
    individs[n_individs].genom[n1] = individs[n].genom[n2];
    individs[n_individs].genom[n2] = c;

    n_individs++;
    if (n_individs == max_individs)
        individs = (struct Individ*)realloc(individs, sizeof(struct Individ) * max_individs);

}

static struct DxfFile* generate_dataset(struct DxfFile *dxf_files, int f_count, int *dataset_size)
{
    int i, j, k;
    int size;
    struct DxfFile *dataset;

    size = 0;
    for (i = 0; i < f_count; i++) {
        size += dxf_files[i].how_many;
    }

    *dataset_size = size;
    dataset = (struct DxfFile*)malloc(sizeof(struct DxfFile) * size); 
    
    for (i = 0, k = 0; i < f_count; i++) {
        for (j = 0; j < dxf_files[i].how_many; j++, k++) {
            dataset[k] = filedup(dxf_files[i]);
        }
    }

    return dataset;
}

static int calculate_individ_height(struct Individ individ, struct DxfFile *dataset, int draw)
{
	int i, j, k, m, genom_size; 
	int max_position, positioned;
	double curr_height;
    double angle_step;
	struct Position *positions;
	
	max_position = individ.genom_size;
	positions = (struct Position*)malloc(sizeof(struct Position) * max_position);
	
    curr_height = 0;

	positioned = 0;
    
    angle_step = 30;
	for (i = 0; i < individ.genom_size; i++) {
		int res, index;
		double min_length, x, y, angle, min_angle;
		struct DxfFile curr_file;
					
        index = individ.genom[i];
		min_length = -1;
	    
        was_placed = 0;
  //      init_mask((int)width, (int)height);
        for (angle = 0.0; angle < 360; angle += angle_step) {
           // printf("i=%d index=%d\n", i, index);
            curr_file = filedup(dataset[index]);
            if (angle > 0) 
                rotate_polygon(&curr_file, angle);
           
	    	for (x = 0.0; x <= width; x += 1.0) {
	    		res = 0;
    			for (y = trunc(curr_height + 1.0); y >= 0; y -= 1.0) {
			    	double g_x, g_y, tmp_length, x_pos, y_pos;
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
                
                    x_pos = x;
                    y_pos = y;

                //    if (x_pos - 1.0 >= 0 && mask[(int)(x_pos - 1.0)][(int)(y_pos - 1.0)] != 1)
                    recursive_move_x(&x_pos, &y_pos, &min_length, &curr_file, positions, positioned);
                    	
		    		if (curr_file.y_max + y > height)				
			    		continue;

    				g_x = curr_file.polygon.gravity_center.x + x_pos;
	    			g_y = curr_file.polygon.gravity_center.y + y;
		    		tmp_length  = sqrt(pow(g_x, 2) + pow(g_y, 2));
                			
                    if (x_pos + curr_file.x_max > width)
                        continue;

    				if (min_length == -1 || tmp_length < min_length) {
                        was_placed = 1;
    					min_length = tmp_length;
                        min_angle = angle;
	    				positions[positioned].file = curr_file;
		    			positions[positioned].x = x_pos;
			    		positions[positioned].y = y;
    				} else if (tmp_length == min_length && positions[positioned].y > y) {
                        min_angle = angle;
	    				positions[positioned].file = curr_file;
		    			positions[positioned].x = x_pos;
			    		positions[positioned].y = y;
    				}
                
                    if (y == 0) {
                        x = width * 2;
                        break;
                    }
	    		}
    		}
        }
        
    //    free_mask(mask, (int)width, (int)height);

        if (!was_placed) 
            continue;

		positioned++;
            
		printf("positioned = %d angle=%f %s\n", positioned, min_angle, positions[positioned - 1].file.path);
		if (positioned == max_position) {					
			max_position *= 2;
			positions = (struct Position*)realloc(positions, sizeof(struct Position) * max_position);
		}

        if (curr_height < positions[positioned - 1].file.y_max + positions[positioned - 1].y);
            curr_height = positions[positioned - 1].file.y_max + positions[positioned - 1].y;

	}
    
    if (positioned < individ.genom_size) 
        curr_height = height * 2;
    min_height = (min_height < curr_height)? min_height : curr_height;
	
	printf("positioned = %d genom_szie=%d \n", positioned, individ.genom_size);
	n_positioned = positioned;
    
    if (draw)
        draw_nested(positions);

    return curr_height;
}

static void calculate_fitness()
{
    int i;
    double avg;
    
    avg = 0.0;
    for (i = 0; i < n_individs; i++)
        avg += individs[i].height;

    avg /= n_individs;

    for (i = 0; i < n_individs; i++) {
        /*if (individs[i].height = HUGE_VAL) {
            individs[i].fitness = HUGE_VAL;
            continue; 
        }*/
        double alpha, beta, sigma;
        alpha = avg / (avg - min_height);
        beta = (-1) * min_height * avg / (avg - min_height);
        sigma = avg;
        individs[i].fitness = alpha * individs[i].height + beta + sigma;
    }
}

static int individs_cmp(const void *indiv1, const void *indiv2)
{
    struct Individ *individ1, *individ2;

    individ1 = (struct Individ*)indiv1;
    individ2 = (struct Individ*)indiv2;

    if (individ1->fitness > individ2->fitness) 
        return 1;
    if (individ1->fitness < individ2->fitness)
        return -1;
    else
        return 0;
}

static int individs_equal(struct Individ individ1, struct Individ individ2)
{
    int i, equal;
    
    equal = 1;
    for (i = 0; i < individ1.genom_size; i++) {
        equal = (individ1.genom[i] == individ2.genom[i])? 1 : 0;
        if (!equal)
            break;
    }

    return equal;
}

static int crossover()
{
    int i, j, count, equal, res;
    int g1, g2, i1, i2;

    res = 0;

    i1 = 0;
    i2 = 1;

    individs[n_individs].genom = (int*)malloc(sizeof(int) * individs[i1].genom_size);
    individs[n_individs].genom_size = individs[i1].genom_size;

    srand(time(NULL));
    g1 = (int)rand % (individs[i1].genom_size - 1);
    g2 = g1 + 1;;

    individs[n_individs].genom[g1] = individs[i1].genom[g1];
    individs[n_individs].genom[g2] = individs[i1].genom[g2];
    
    count = 0;
    for (i = 0; i < individs[i1].genom_size; i++) {
        if (count == g1 || count == g2) {
            count++;
            i--;
            continue;
        }

        if (individs[i2].genom[i] == individs[i1].genom[g1] || individs[i2].genom[i] == individs[i1].genom[g2])
            continue;

        individs[n_individs].genom[count] = individs[i2].genom[i];
        count++;
    }

    equal = 0;
    for (i = 0; i < n_individs; i++) {
        equal = individs_equal(individs[i], individs[n_individs]);

        if (equal)
            break;
    }

    if (!equal) {
        res += 1;
        n_individs++;
        individs[n_individs].genom = (int*)malloc(sizeof(int) * individs[i1].genom_size);
        individs[n_individs].genom_size = individs[i1].genom_size;
    }   

    individs[n_individs].genom[g1] = individs[i2].genom[g1];
    individs[n_individs].genom[g2] = individs[i2].genom[g2]; 
    
    count = 0;
    for (i = 0; i < individs[i2].genom_size; i++) {
        if (count == g1 || count == g2) {
            count++;
            i--;
            continue;
        }

        if (individs[i1].genom[i] == individs[i2].genom[g1] || individs[i1].genom[i] == individs[i2].genom[g2])
            continue;

        individs[n_individs].genom[count] = individs[i1].genom[i];
        count++;
    }

    equal = 0;
    for (i = 0; i < n_individs; i++) {
        equal = individs_equal(individs[i], individs[n_individs]);

        if (equal)
            break;
    }

    if (!equal) {
        res += 1;
        n_individs += 1;
    }

   return res;
  
}

void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double w, double h)
{
    int i, j, k, dataset_size;
    struct DxfFile *dataset;
    printf("box_height=%f\n", height);
    min_height = height;
    max_individs = 256;
    n_individs = 0;
    width = w;
    height = h;

    individs = (struct Individ*)malloc(sizeof(struct Individ) * max_individs);
    dataset = generate_dataset(dxf_files, f_count, &dataset_size);
    printf("dataset_size=%d\n", dataset_size);
    qsort(dataset, dataset_size, sizeof(struct DxfFile), dxf_cmp);
    max_genom = dataset_size;
    
    generate_first_individ(dataset, dataset_size);

   // return;
    
    for (i = 0; i < 1; i++) {
        mutate_individ(0, individs[0].genom_size);
        for (j = 0; j < n_individs - 1; j++) {
            if (individs_equal(individs[j], individs[n_individs - 1])) {
                i--;
                n_individs--;
                break;
            }
        }
    }

    for (i = 0; i < n_individs; i++) {
        individs[i].height = calculate_individ_height(individs[i], dataset, 0);
    }
    
    printf("\n\n");
    printf("min_height=%f\n", min_height);
    calculate_fitness();
    for (i = 0; i < 10; i++) {
        int res;
        qsort(individs, n_individs, sizeof(struct Individ), individs_cmp);
        for (j = 0; j < n_individs; j++) {
            printf("height=%f fitness=%f\n", individs[j].height, individs[j].fitness);
        }
        printf("\n");
        
        res = crossover();
        
        if (res == 0) {
            printf("ALERT CROSSOVER \n");
            int equal = 1;
            while (equal) {
                mutate_individ(n_individs / 2, individs[n_individs / 2].genom_size);
                for (j = 0; j < n_individs - 1; j++) {
                    equal = individs_equal(individs[j], individs[n_individs - 1]);
                    if (equal) {
                        n_individs--;
                        break;
                    } else {
                        res = 1;
                    }
                }
            }
        }


        for(j = 0; j < n_individs; j++) {
            printf("gen%d: ", j);
            for (k = 0; k < individs[j].genom_size; k++) {
                printf("%d ", individs[j].genom[k]);
            }
            printf("\n");
        }

        individs[n_individs - 1].height = calculate_individ_height(individs[n_individs - 1], dataset, 0);
        if (res == 2)
            individs[n_individs - 2].height = calculate_individ_height(individs[n_individs - 2], dataset, 0);
        calculate_fitness();
    }

    qsort(individs, n_individs, sizeof(struct Individ), individs_cmp);
    printf("\n\n\n\n RESULT MIN_HEIGHT=%f FITNESS=%f\n", individs[0].height, individs[0].fitness);
    calculate_individ_height(individs[0], dataset, 1);
}
