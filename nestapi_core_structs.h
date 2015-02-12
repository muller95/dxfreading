enum {
	DXF_TYPE_LINE,
	DXF_TYPE_SPLINE,
};

struct PointD {
	double x, y;
};

struct Rectangle {
	double x, y, width, height;
};

struct Polygon {
	struct PointD *points;
	struct PointD gravity_center;
	int n_points;
};

struct DxfPrimitive {
	struct PointD *points;
	int type;
};

struct TextPrimitive {
	char **lines;
};

struct DxfFile {
	char *path;
	struct TextPrimitive *text_primitives;
	struct DxfPrimitive *primitives;
	struct Polygon polygon;
	int n_primitives, n_types; 
	int max_types, max_lines; 
	int *n_controldots, *str_count, *types;
	double m_height, m_width; 
	double x_min, x_max, y_min, y_max;
};

struct Position {
	struct DxfFile file;
	double x, y;
	int i;
};

struct Level {
	double from_x, to_x, y;
};

struct Individ {
    int *genom;
    int genom_size;
    double height, fitness;
};

struct NfpPoint {
    struct PointD point;
    struct NfpPoint *next;
};
