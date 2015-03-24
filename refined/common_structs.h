
/* Data structures for all functions */

#define DXF_TYPE_LINE 0
#define DXF_TYPE_SPLINE 1

struct Point {
  double x;
  double y;
}

struct DxfPrimitive {
  struct Point *points;
  int type;
  int controldots;
  struct DxfPrimitive *next;
}

struct LinearPrimitive {
  struct Point *points;
  int type;
}

struct DxfFile {
  char *path;
  int quant;
  struct DxfPrimitive *primitives;
/*  Не помню, как назывались, и для чего нужны
  double my_height;
  double my_width;
*/
  struct Point min;
  struct Point max;
}

