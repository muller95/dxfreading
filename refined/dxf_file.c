#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common_structs.h"

#define ENTITIES "ENTITIES\n"
#define ENDSECS "ENDSECS\n"


void
dxf_file_create(char *path, struct DxfFile *df, int quant)
{

  assert(path != NULL);
  assert(df != NULL);
  assert(quant > 0);

  FILE *fp;
  char *line;
  int line_length, in_entities;
  ssize_t ch_read;
  struct DxfPrimitive *dp;

  fp = fopen(path, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  df->primitives = NULL;
  df->quant = quant;
  df->path = (char *)calloc(strlen(path)+1, sizeof(char));
  memcpy(df->path, path, strlen(path)+1);

  line = NULL;
  line_length = 0;
  in_entities = 0;

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {

    if (strcmp(line, ENTITIES)) {
      in_entities = 1;
      continue;
    } else if (strcmp(line, ENDSECS)) {
      in_entities = 0;
      continue;
    }

    if (in_entities) {
      /* Do some magic fancy stuff w/ entities */
    }
  }

  free(line);
  fclose(fp);
}


void
dxf_file_destroy(struct DxfFile *df)
{
  assert(df != NULL);

  struct DxfPrimitive *tmp, *cur;

  free(df->path);

  cur = df->primitives;
  while (cur != NULL) {
    tmp = cur;
    cur = cur->next;
    free(tmp);
  }

  df = NULL;
  
}
