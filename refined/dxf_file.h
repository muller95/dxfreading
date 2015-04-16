#ifndef _DXF_FILE_H_
#define _DXF_FILE_H_

#include "dxf_structs.h"
#include "dxf_common_codes.h"

struct DxfFile * dxf_file_open(const char *path);
void dxf_file_close(struct DxfFile *df);

#endif /* _DXF_FILE_H_ */
