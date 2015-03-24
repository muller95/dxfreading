#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common_structs.h"


void dxf_file_create(char *path, struct DxfFile *df, int quant)
{

  assert(path != NULL);
  assert(df != NULL);
  assert(quant > 0);

  FILE *fp;
  char *line;
  int line_length;
  ssize_t ch_read;

  fp = fopen(path, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  df->quant = quant;
  df->path = (char *)calloc(strlen(path)+1, sizeof(char))
  memcpy(df->path, path, strlen(path));

  line = NULL;
  line_length = 0;

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    /* Do some magic fancy stuff w/ lines */
  }

  free(line);
  fclose(fp);
}


void dxf_file_destroy(struct DxfFile *df)
{
  assert(df != NULL);

  DxfPrimitive *tmp, *cur;

  free(path);

  cur = df->primitives;
  while (cur != NULL) {
    tmp = cur;
    cur = cur->next;
    free(tmp);
  }
  
}
