#include "nestapi_core_structs.h"

void dxf_file_new(struct DxfFile *dxf_file, char *path, int quant);
void show_dxf_file(struct DxfFile *dxf_file);
void nest_dxf(struct DxfFile *fdxf_files, int f_count, int width, int height);
void start_nfp_nesting(struct DxfFile *dxf_files, int f_count, double width, double height);
int test_function();



