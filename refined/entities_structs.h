
/* DXF entities types definitions, used to parse DXF file */

#ifndef _ENTITIES_STRUCTS_H_
#define _ENTITIES_STRUCTS_H_

#define ENTITIES "ENTITIES"
#define ENDSEC "ENDSEC"

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
  char *cp_x;  /* cp == control point */
  char *cp_y;
  char *cps_quant; 
};
extern struct entity_spline_t entity_spline;

#endif /* _ENTITIES_STRUCTS_H_ */
