#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gtk/gtk.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"
#include "dxf_geometry.h"
#include "cross_check_funcs.h"
#include "nfp_insert_funcs.h"

static int n_positioned;
static int max_individs, n_individs, max_genom;
static double min_height;

struct Individ *individs;
struct NfpPoint *head;

void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double width, double height);

static void init_nfp(double width, double height);
static int crossover();
static int individs_equal(struct Individ individ1, struct Individ individ2);
static int individs_cmp(const void *individ1, const void *individ2);
static void calculate_fitness();
static int calculate_individ_height(struct Individ individ, struct DxfFile *dataset, double width, double height, int draw);
static struct DxfFile* generate_dataset(struct DxfFile *dxf_files, int f_count, int *dataset_size);
static void mutate_individ(int n, int genom_size);
static int left_hole_nesting(struct DxfFile *dataset, struct Position *positions, int *max_position, int positioned, int dataset_size, double height);
static void generate_first_individ(struct DxfFile *dataset, int dataset_size, double width, double height);
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


static void generate_first_individ(struct DxfFile *dataset, int dataset_size, double width, double height)
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
		int res, was_placed;
        struct NfpPoint *curr, *next;
		double min_length;
        struct PointD min_offset;
		struct DxfFile curr_file;
					
		curr_file = filedup(dataset[i]);
		min_length = -1;
	    was_placed = 0;
        
        curr = head;
        next = curr->next;
        while(curr->next != NULL) {
            struct PointD move_vector, start, end;
             
            start = curr->point;
            end = next->point;
            
       //     printf("start x=%f y=%f\n", start.x, start.y);

            curr = next;
            next = next->next;
            
            move_vector.x = end.x - start.x;
            move_vector.y = end.y - start.y;

            if (fabs(move_vector.x) > fabs(move_vector.y) & fabs(move_vector.x) > 1.0) {
                move_vector.y /= fabs(move_vector.x);
                move_vector.x /= fabs(move_vector.x);
            } else if (fabs(move_vector.y) > fabs(move_vector.x) && fabs(move_vector.y) > 1.0) {
                move_vector.x /= fabs(move_vector.y);
                move_vector.y /= fabs(move_vector.y);
            }

            for (k = 0; k < curr_file.polygon.n_points; k++) {
                double g_x, g_y, tmp_length;
                int flag;      
                struct PointD offset, ref_p;
                
                ref_p = curr_file.polygon.points[k];

                offset.x = start.x - ref_p.x;
                offset.y = start.y - ref_p.y;

                ref_p.x += offset.x;
                ref_p.y += offset.y;
                flag = 1;
                while (flag) {
                    if (fabs(ref_p.x - end.x) <= 0.00001 && fabs(ref_p.y - end.y) <= 0.00001)
                        flag = 0;
                     
 
                        
                    if (offset.x < 0 || offset.y < 0) {
                        offset.x += move_vector.x;
                        offset.y += move_vector.y;
                        ref_p.x += move_vector.x;
                        ref_p.y += move_vector.y;
                        continue;
                    }

                    if (curr_file.y_max + offset.x >= width || curr_file.y_max + offset.y >= height) {
                        offset.x += move_vector.x;
                        offset.y += move_vector.y;
                        ref_p.x += move_vector.x;
                        ref_p.y += move_vector.y;
                        if (ref_p.x == end.x && ref_p.y == end.y)
                            flag = 0;
                        continue;
                    }
                    
                    if (cross_check_nfp(curr_file, offset, head)) {
                        offset.x += move_vector.x;
                        offset.y += move_vector.y; 
                        ref_p.x += move_vector.x;
                        ref_p.y += move_vector.y;
                        continue;
                    }

                    if (out_of_nfp(curr_file, head, offset, width)) {
                        offset.x += move_vector.x;
                        offset.y += move_vector.y; 
                        ref_p.x += move_vector.x;
                        ref_p.y += move_vector.y;
                        continue;
                    }

                    g_x = curr_file.polygon.gravity_center.x + offset.x;
				    g_y = curr_file.polygon.gravity_center.y + offset.y;
    				tmp_length  = sqrt(pow(g_x, 2) + pow(g_y, 2));			

	    			if (min_length == -1 || tmp_length < min_length) {
                        was_placed = 1;
			    		min_length = tmp_length;
                        min_offset.x = offset.x;
                        min_offset.y = offset.y;
				    	positions[positioned].file = curr_file;
					    positions[positioned].x = offset.x;
    					positions[positioned].y = offset.y;
	    			} else if (tmp_length == min_length && positions[positioned].y > offset.y) {
		    			min_offset.x = offset.x;
                        min_offset.y = offset.y;
                        positions[positioned].file = curr_file;
			    		positions[positioned].x = offset.x;
				    	positions[positioned].y = offset.y;
    				}
                    
                //    printf("x=%f y=%f end.x=%d end.y=%f\n", ref_p.x, ref_p.y, end.x, end.y);
                 //   getchar();
                    
                    offset.x += move_vector.x;
                    offset.y += move_vector.y;
                    ref_p.x +=  move_vector.x;
                    ref_p.y += move_vector.y;
                }
            }
        }

        if (!was_placed) 
            continue;
            
        min_offset.x = positions[positioned].x;
        min_offset.y = positions[positioned].y;
		positioned++;
        n_positioned = positioned; 
      //  if (positioned == 1)
        //    positioned = left_hole_nesting(dataset, positions, &max_position, positioned, dataset_size, height);

        genom_size = individs[0].genom_size;

           
		printf("positioned = %d n %s\n", positioned, positions[positioned - 1].file.path);
       
        update_nfp(curr_file, head, min_offset);
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

static struct DxfFile* generate_dataset(struct DxfFile *dxf_files, int f_count, int *dataset_size)
{
    int i, j, k;
    int size, n_files;
    struct DxfFile *dataset;
    
    n_files = 2;
    size = n_files * f_count;
    *dataset_size = size;
    dataset = (struct DxfFile*)malloc(sizeof(struct DxfFile) * size); 
    
    for (i = 0, k = 0; i < f_count; i++) {
        for (j = 0; j < n_files; j++, k++) {
            dataset[k] = filedup(dxf_files[i]);
        }
    }

    return dataset;
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

static void init_nfp(double width, double height) 
{
    struct NfpPoint *p1, *p2, *p3, *p4;
    
    p4 = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
    p4->next = NULL;
    p4->point.x = width - 1;
    p4->point.y = height - 1;

    p3 = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
    p3->next = p4;
    p3->point.x = width - 1;;
    p3->point.y = 0;
    
    p2 = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
    p2->next = p3;
    p2->point.x = 0;
    p2->point.y = 0;
    
    p1 = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
    p1->next = p2;
    p1->point.x = 0;
    p1->point.y = height - 1;
    
    head = p1;
}

void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double width, double height)
{
    int i, j, k, dataset_size;
    struct DxfFile *dataset;
    printf("box_height=%f\n", height);
    min_height = height;
    max_individs = 256;
    n_individs = 0;

    individs = (struct Individ*)malloc(sizeof(struct Individ) * max_individs);
    dataset = generate_dataset(dxf_files, f_count, &dataset_size);
    printf("dataset_size=%d\n", dataset_size);
    qsort(dataset, dataset_size, sizeof(struct DxfFile), dxf_cmp);
    max_genom = dataset_size;
    init_nfp(width, height);

    generate_first_individ(dataset, dataset_size, width, height);
    
   /* for (i = 0; i < 1; i++) {
        mutate_individ(0, individs[0].genom_size);
        for (j = 0; j < n_individs - 1; j++) {
            if (individs_equal(individs[j], individs[n_individs - 1])) {
                i--;
                n_individs--;
                break;
            }
        }
    }

    for (i = 1; i < n_individs; i++) {
        individs[i].height = calculate_individ_height(individs[i], dataset, width, height, 0);
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

        individs[n_individs - 1].height = calculate_individ_height(individs[n_individs - 1], dataset, width, height, 0);
        if (res == 2)
            individs[n_individs - 2].height = calculate_individ_height(individs[n_individs - 2], dataset, width, height, 0);
        calculate_fitness();
    }

    qsort(individs, n_individs, sizeof(struct Individ), individs_cmp);
    printf("\n\n\n\n RESULT MIN_HEIGHT=%f FITNESS=%f\n", individs[0].height, individs[0].fitness);
    calculate_individ_height(individs[0], dataset, width, height, 1);*/
}


