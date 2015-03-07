#include <stdlib.h>
#include <string.h>
#include "nestapi_core_structs.h"

struct DxfFile filedup(struct DxfFile source)
{
    int i, j;
    struct DxfFile dest;
    
    dest.path = strdup(source.path);
    dest.how_many = source.how_many;
    dest.n_primitives = source.n_primitives;
    dest.n_controldots = (int*)malloc(sizeof(int) * source.n_primitives);
    dest.primitives = (struct DxfPrimitive*)malloc(sizeof(struct DxfPrimitive) * source.n_primitives);

    for (i = 0; i < source.n_primitives; i++) {
        dest.primitives[i].points = (struct PointD*)malloc(sizeof(struct PointD) * source.n_controldots[i]);
        dest.n_controldots[i] = source.n_controldots[i];
        dest.primitives[i].type = source.primitives[i].type;
        for (j = 0; j < source.n_controldots[i]; j++) {
            dest.primitives[i].points[j].x = source.primitives[i].points[j].x;
            dest.primitives[i].points[j].y = source.primitives[i].points[j].y;
        }
    }

    dest.polygon.n_points = source.polygon.n_points;
    dest.polygon.points = (struct PointD*)malloc(sizeof(struct PointD) * source.polygon.n_points);
    dest.polygon.gravity_center.x = source.polygon.gravity_center.x;
    dest.polygon.gravity_center.y = source.polygon.gravity_center.y;

    for (i = 0; i < source.polygon.n_points; i++) {
        dest.polygon.points[i].x = source.polygon.points[i].x;
        dest.polygon.points[i].y = source.polygon.points[i].y;
    }
    
    dest.n_types = source.n_types;
    dest.types = (int*)malloc(sizeof(int) * source.n_types);

    for (i = 0; i < source.n_types; i++) 
        dest.types[i] = source.types[i];

    dest.max_types = source.max_types;

    dest.m_height = source.m_height;
    dest.m_width = source.m_width;

    dest.x_max = source.x_max;
    dest.x_min = source.x_min;
    dest.y_max = source.y_max;
    dest.y_min = source.y_min;

    return dest;
}
