
/* Data structures for all functions */

#ifndef _COMMON_STRUCTS_H_
#define _COMMON_STRUCTS_H_

#include "entities_structs.h"

struct Point {
  double x;
  double y;
};

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

struct Primitive {
  char *type;
  void *data;
  struct Primitive *next;
};

struct DxfFile {
  char *path;
  int file_quant;
  struct Primitive *primitives;
  double height;
  double width;
  struct Point min;
  struct Point max;
};

#endif /* _COMMON_STRUCTS_H_ */
