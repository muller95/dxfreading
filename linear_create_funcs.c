#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nestapi_core_structs.h"

struct LinearFigure liearise_dxf_file(struct DxfFile *dxf_file) 
{
    int i, j, k;
    struct LinearFigureD lin_fig;

    lin_fig.primitives = (struct LineD*)malloc(sizeof(struct LinearPrimitiveD) * dxf_file->n_primitives);

    for (int i = 0; i < dxf_file->n_primitives; i++) {
       if (dxf_file->types[i] == DXF_TYPE_SPLINE) {
           int max_points = 1024;
           double t, step;
           struct PointD *tmp_points;

           lin_fig.primitives[i].points = (struct PointD*)malloc(sizeof(struct PointD) * max_points);            
           step = 0.2;
           for (t = 0; t <= 1; t += step) {
               tmp_points = (struct PointD*)malloc(sizeof(struct PointD) * dxf_files->n_controldots[i]);
           }
       }
    }
}
