/*DXF_TYPE_LINE is a broken line from N points
DXF_TYPE_SPLINE is a bezier curve with N controldots*/
enum {
	DXF_TYPE_LINE,
	DXF_TYPE_SPLINE,
};

struct PointD {
	double x, y;
};



struct TextPrimitive {
	char **lines;
};

struct Polygon {
	struct PointD *points;
	struct PointD gravity_center;
	int n_points;
};

struct DxfPrimitive {
	//points of primitive
	struct PointD *points;
	//type and number of control dots
	int type, n_controldots;
};

struct DxfFile {
	//path to file
	char *path;
	//primitives
	struct DxfPrimitive *primitives;
	struct Polygon polygon;
	//number of primitives
	int n_primitives;
    int how_many; 
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

struct LinearFigureD {
    struct LinearPrimitiveD *primitives;
    int n_primitives;
};
