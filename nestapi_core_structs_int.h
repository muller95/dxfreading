struct PointI {
	int x, y;
};

struct PolygonI {
	struct PointI *points;
	struct PointI gravity_center;
	int n_points;
};

struct DxfPrimitiveI {
	struct PointI *points;
	int type;
};

struct DxfFileI {
	char *path;
	struct TextPrimitive *text_primitives;
	struct DxfPrimitiveI *primitives;
	struct PolygonI polygon;
	int n_primitives, n_types;
    int how_many; 
	int max_types, max_lines; 
	int *n_controldots, *str_count, *types;
	int m_height, m_width; 
	int x_min, x_max, y_min, y_max;
};

struct PositionI {
	struct DxfFileI file;
	int x, y;
	int i;
};
