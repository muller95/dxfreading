
/* Data structures for all functions */

#define DXF_TYPE_LINE 0
#define DXF_TYPE_SPLINE 1

struct Point {
	double x, y;
};

struct DxfPrimitive {
  struct Point *points;
  int type;
  int npoints;
  struct DxfPrimitive *next;
};

struct LinearPrimitive {
  struct Point *points;
  int npoints;
  int type;
};

struct DxfFile {
  char *path;
  int quant;
  struct DxfPrimitive *primitives;
  double height, width;
  struct Point min, max;
};

