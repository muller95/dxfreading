
/* Data structures for all functions */

#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include "entities_structs.h"

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

/*
struct CircleData {
  
};
*/

struct LinearData {
  struct Point *points;
  int point_quant;
};

/* ---------- */

struct Primitive {
  char *type;
  void *data;
  struct Primitive *next;
};

struct DxfFile {
  char *resolved_path;
  struct Primitive *primitives;
  int primitives_quant;
};

struct Linear {
  char *id;
  struct Primitive *primitives;
  struct Point mass_center;
  struct Point min;
  struct Point max;
};

#endif /* _COMMON_STRUCTS_H_ */
