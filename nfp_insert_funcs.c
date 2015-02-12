#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nestapi_core_structs.h"
#include "dxf_work_functions.h"
#include "dxf_geometry.h"
#include "cross_check_funcs.h"

struct NfpPoint *left_start, *right_end;


void update_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset);


static void find_extreme(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int *li, int *ri);
static void find_extreme2(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int *li, int *ri);
static void insert_chain_to_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int li, int ri);


static void find_extreme(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int *li, int *ri)
{
    int i, first;
    struct NfpPoint *curr, *next;
    struct PointD left_point, right_point;
    
    first = 1;
    curr = head;
    next = curr->next;

    while (curr->next != NULL) {
        struct PointD p1, p2;
        double k, b;
        double xl, xr, yt, yb;
        
        p1 = curr->point;
        p2 = next->point;

        xl = (p1.x < p2.x)? p1.x : p2.x;
        xr = (p1.x > p2.x)? p1.x : p2.x;
        yb = (p1.y < p2.y)? p1.y : p2.y;
        yt = (p1.y > p2.y)? p1.y : p2.y; 
  
        determine_line(&k, &b, p1, p2);        
        for (i = 0; i < curr_file.polygon.n_points - 1; i++) {
            struct PointD cp;
            double y;

            cp = curr_file.polygon.points[i];
            cp.x += offset.x;
            cp.y += offset.y;
    
            if (cp.x < xl || cp.x > xr)
                continue;
            
            if (fabs(k) == INFINITY) {
                y = cp.y;
            }
            else
                y = k * cp.x + b;

         /*   if (y >= yt || y <= yb) 
                continue;*/


            if (fabs(cp.y - y) > 0.1)
                continue;

           
            
            if (first) {
                *li = i;
                left_point = cp;
                left_start = curr;

                *ri = i;
                right_point = cp;
                right_end = next;
              //  printf("first left: x=%f y=%f\n", left_point.x, left_point.y);
                first = 0;
            }

            if (cp.x < left_point.x || (left_point.x == cp.x && cp.y > left_point.y)) {
                *li = i;
                left_point = cp;
                left_start = curr;
            }
            if (cp.x > right_point.x || (right_point.x == cp.x && cp.y > right_point.x)) {
                *ri = i;
                right_point = cp;
                right_end = next;
            }
        }
        curr = next;
        next = next->next;
    }
}


static void find_extreme2(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int *li, int *ri)
{
    int i;
    struct NfpPoint *curr, *next, *prev;
    struct PointD left_point, right_point;

    left_point = curr_file.polygon.points[*li];
    left_point.x += offset.x;
    left_point.y += offset.y;
    right_point = curr_file.polygon.points[*ri];
    right_point.x += offset.x;
    right_point.y += offset.y;

    for (i = 0; i < curr_file.polygon.n_points - 1; i++) {
        struct PointD p1, p2;
        double k, b;
        double xl, xr, yb, yt;

        p1 = curr_file.polygon.points[i];
        p1.x += offset.x;
        p1.y += offset.y;
        p2 = curr_file.polygon.points[i + 1];
        p2.x += offset.x;
        p2.y += offset.y;

        xl = (p1.x < p2.x)? p1.x : p2.x;
        xr = (p1.x > p2.x)? p1.x : p2.x;
        yb = (p1.y < p2.y)? p1.y : p2.y;
        yt = (p1.y > p2.y)? p1.y : p2.y; 

        curr = head;
        determine_line(&k, &b, p1, p2); 
        while (curr != NULL) {
            struct PointD cp;
            double y;

            cp = curr->point;
            prev = curr;
            curr = curr->next;

            if (cp.x < xl || cp.x > xr)
                continue;
 
            if (fabs(k) == INFINITY) {
                y = cp.y;
            }
            else
                y = k * cp.x + b;


            /*if (y >= yt || y <= yb) 
                continue;*/

            if (fabs(cp.y - y) > 3)
                continue;
            
          //  printf("cp.x=%f cp.y=%f lp.x=%f lp.y=%f\n", cp.x, cp.y, left_point.x, left_point.y);
          //  getchar();

            if (cp.x < left_point.x || (left_point.x == cp.x && cp.y > left_point.y)) {
                if (p1.y > p2.y || (p1.y == p2.y && p1.x < p2.x)) {
                //    printf("i p1.y=%f p2.y=%f\n", p1.y, p2,y);
                    *li = i;
                }
                else if (p2.y > p1.y || (p2.y == p1.y && p2.x < p1.x)) {
                 //   printf("i + 1 p1.y=%f p2.y=%f\n", p1.y, p2,y);
                    *li = i + 1;
                }

                left_point = cp;
                left_start = prev;
            //    printf("here left\n");
            }

            if (cp.x > right_point.x || (right_point.x == cp.x && cp.y > right_point.y)) {
                if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x))
                    *ri = i;
                else if (p2.y > p1.y || (p2.y == p1.y && p2.x > p1.x))
                    *ri = i + 1;

                right_point = cp;
                right_end = prev;
             //   printf("here right\n");
            }           
        }
    }
}

static void insert_chain_to_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, int li, int ri)
{
   int i, step, can_change;
   struct PointD p, left_point, right_point;
   struct NfpPoint *curr;
   
   i = li;

   p = curr_file.polygon.points[i];
   p.x += offset.x;
   p.y += offset.y;
   curr = left_start;
   
   if (li == curr_file.polygon.n_points - 1 || li == 0) {
       int n;
       n = curr_file.polygon.n_points;
       if (curr_file.polygon.points[1].y > curr_file.polygon.points[n - 2].y)
           step = 1;
       else
           step = -1;
   } else {
        if (curr_file.polygon.points[li + 1].y > curr_file.polygon.points[li - 1].y)
           step = 1;
       else
           step = -1;
   }
   
   if (p.x != curr->point.x || p.y != curr->point.y) {
        struct NfpPoint *new_node;
        new_node = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
        new_node->point.x = p.x;
        new_node->point.y = p.y;
        curr->next = new_node;
        curr = new_node;
   }
   
   i += step;
   can_change = 1;
   while (can_change) {
        struct NfpPoint *new_node;
        //i += step;
         
        if (i == curr_file.polygon.n_points)
            i = 1;
        else if (i == -1)
            i = curr_file.polygon.n_points - 2; 
        
     //   printf("i=%d\n", i);
      //  getchar();
        p = curr_file.polygon.points[i];
        p.x += offset.x;
        p.y += offset.y;

        new_node = (struct NfpPoint*)malloc(sizeof(struct NfpPoint));
        new_node->point.x = p.x;
        new_node->point.y = p.y;

//        printf("x=%f\n", new_node->point.x);

        if (i == ri) {
            if (p.x == right_end->point.x && p.y == right_end->point.y) {
                curr->next = right_end;
            } else {
                curr->next = new_node;
   //             curr->point.x =p.x;
     //           curr->point.y = p.y;
               // new_node->next = right_end;
               curr = curr->next;
               curr->next = right_end;
            }
            can_change = 0;
            break;
        }

        curr->next = new_node;
        curr = new_node;
        i += step;
   }
   printf("\n\n");
   curr = head;
/*   while (curr != NULL) {
       printf("x=%f y=%f\n", curr->point.x, curr->point.y);
       curr = curr->next;
   }*/
//   printf("WAITING\n");
 //  getchar();
}

void update_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset)
{
    int li, ri;
    find_extreme(curr_file, head, offset, &li, &ri);
    find_extreme2(curr_file, head, offset, &li, &ri);
  /*  printf("lx=%f rx=%f\n", curr_file.polygon.points[li].x + offset.x, curr_file.polygon.points[ri].x + offset.x);
    printf("left_start x=%f y=%f\n", left_start->point.x, left_start->point.y);
    printf("right_end x=%f y=%f\n", right_end->point.x, right_end->point.y);
    printf("li=%d ri=%d\n", li, ri);*/
    //getchar();
    insert_chain_to_nfp(curr_file, head, offset, li, ri);
}

