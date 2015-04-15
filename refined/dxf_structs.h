
/* Data structures for all functions */

#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include "entities_structs.h"

#define NEW_ENTITY "0"
#define HANDLE "5"
#define ENTITY_DATA "AcDbEntity"
#define DIVIDER "100"

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
  int cps_quant;
  struct Point *points; 
};

struct LinearData {
  char *type;
  struct Point *points;
  int point_quant;
};

union EntityData {
  char *type;
  struct LineData line;
  struct SplineData spline;
};

/* ---------- */

struct Header {
 /* header stuff */ 
};

struct Entity {
  union EntityData *data;
  struct Entity *next;
};

struct DxfFile {
  char *resolved_path;
  struct Header header;
  struct Entity *entities;
  int entities_quant;
};

struct Linear {
  char *id;
  struct Entity *entities;
  struct Point mass_center;
  struct Point min;
  struct Point max;
};

#endif /* _COMMON_STRUCTS_H_ */
