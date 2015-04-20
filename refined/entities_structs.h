
/* DXF entities types definitions, used to parse DXF file */

#ifndef _ENTITIES_STRUCTS_H_
#define _ENTITIES_STRUCTS_H_

struct entity_line_t {
  char *string;
  char *dbsect;
  char *x_begin;
  char *y_begin;
  char *x_end;
  char *y_end;
};
extern struct entity_line_t entity_line;

struct entity_spline_t {
  char *string;
  char *dbsect;
  char *knots_quant;
  char *cps_quant;
  char *fitp_quant; 
  char *cp_x;  /* cp == control point */
  char *cp_y;
};
extern struct entity_spline_t entity_spline;

#endif /* _ENTITIES_STRUCTS_H_ */
