#ifndef _DXF_FILE_H_
#define _DXF_FILE_H_

#include "common_structs.h"

struct DxfFile * dxf_file_open(char *path);
void dxf_file_close(struct DxfFile *df);

#endif /* _DXF_FILE_H_ */
