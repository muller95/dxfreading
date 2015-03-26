
/* DXF entities types definitions, used to parse DXF file */

#ifndef _ENTITIES_STRUCTS_H_
#define _ENTITIES_STRUCTS_H_

#define ENTITIES "ENTITIES"
#define ENDSEC "ENDSEC"

struct entity_line_t {
  char *string;
  char *x_begin;
  char *y_begin;
  char *x_end;
  char *y_end;
};
extern struct entity_line_t entity_line;

struct entity_spline_t {
  char *string;
  char *cp_x;  /* cp == control point */
  char *cp_y;
  char *knot;
  char *knots_quant;  /* <= !!! */
  char *cps_quant; 
};
extern struct entity_spline_t entity_spline;

/* Для чего вообще нужен этот файл, смотри, Вадим
   Что, если тебе в код надо добавить парсинг остальных entities?

struct entity_circle_t {
  char *string;
  char *x;
  char *y;
  char *radius;
};
extern struct entity_circle_t entity_circle;

 Это очень удобный способ задания спецификаций */

#endif /* _ENTITIES_STRUCTS_H_ */
