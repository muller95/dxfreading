
/* Data structures for dxf_file functions */

#ifndef _DXF_STRUCTS_H_
#define _DXF_STRUCTS_H_

#include "entities_structs.h"

#define ENTITY_DATA "AcDbEntity"

struct Point {
  double x;
  double y;
};

/* Entity data structures */

struct LineData {
  char *type;
  struct Point begin;
  struct Point end;
};

struct SplineData {
  char *type;
  /* spline stuff */
};


/* ---------- */

struct Header {
 /* header stuff */ 
};

struct Class {
 /* classes stuff */
};

struct Table {
 /* tables stuff */
};

struct Block {
 /* blocks stuff */
};

union Entity {
  char *type;
  struct LineData line;
  struct SplineData spline;
};

struct Object {
 /* objects stuff */
};

struct Thumbnail {
 /* thumbnail stuff */
};

struct DxfFile {
  char *resolved_path;
  struct Header *header;
  struct Class *classes;
  struct Table *tables;
  struct Block *blocks;
  union Entity *entities;
  struct Object *objects;
  struct Thumbnail *thumbnail;
  int entities_quant;
};

struct Linear {
  char *id;
  int point_quant;
  struct Point *points;
  struct Point mass_center;
  struct Point min;
  struct Point max;
};

#endif /* _DXF_STRUCTS_H_ */
