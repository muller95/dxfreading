
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

/* Primitive data structures */

struct LineData {
  struct Point begin;
  struct Point end;
};

struct SplineData {
  /* spline stuff */
};

struct LinearData {
  struct Point *points;
  int point_quant;
};

/* ---------- */

struct Header {
  
};

struct Entity {
  char *type;
  void *data;
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
  struct Primitive *primitives;
  struct Point mass_center;
  struct Point min;
  struct Point max;
};

#endif /* _COMMON_STRUCTS_H_ */
