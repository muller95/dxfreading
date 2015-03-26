
/* Functions to work with DxfFile */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "dxf_file.h"

static char * strip_str(char *);
static struct Primitive * primitive_create(char *);
static void fill_primitive(struct Primitive *, char *, FILE *);
static struct LineData * get_line(char *, FILE *);
static struct SplineData * get_spline(char *, FILE *);

struct entity_line_t entity_line = {"LINE", "10", "20", "11", "21"};
struct entity_spline_t entity_spline = {"SPLINE", "10", "20", "40", "72", "73"};
/*
struct entity_circle = {"CIRCLE", "10", "20", "40"};
*/

struct DxfFile *
dxf_file_open(char *path, int file_quant)
{

  assert(path != NULL);
  assert(file_quant > 0);

  FILE *fp;
  char *line, *stripped;
  int in_entities;
  size_t line_length;
  ssize_t ch_read;
  struct Primitive *prm, *cur;
  struct DxfFile *df;

  fp = fopen(path, "r");
  if (fp == NULL) {
    return NULL;
  }

  df = (struct DxfFile *)calloc(1, sizeof(struct DxfFile));
  df->primitives = NULL;
  df->file_quant = file_quant;
  df->path = (char *)calloc(strlen(path)+1, sizeof(char));
  memcpy(df->path, path, strlen(path)+1);

  cur = NULL;
  prm = NULL;
  line = NULL;
  line_length = 0;
  in_entities = 0;

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {

    stripped = strip_str(line);
    if (stripped == NULL) {
      return NULL;
    }

    if (strcmp(stripped, ENTITIES) == 0) {
      in_entities = 1;
      continue;
    } else if (strcmp(stripped, ENDSEC) == 0 && in_entities == 1) {
      in_entities = 0;
      break;
    }

    if (in_entities) {
      if (!is_primitive(line)) {
        continue;
      }
      primitive_create(stripped);
      fill_primitive(prm, line, fp);
      if (prm == NULL) {
        return NULL;
      }

      if (df->primitives == NULL) {
        df->primitives = prm;
        cur = prm;
      } else {
        cur->next = prm;
        cur = prm;
      }
    }

  free(stripped);  
  }

  free(line);
  fclose(fp);

  return df;
}


void
dxf_file_close(struct DxfFile *df)
{
  assert(df != NULL);

  struct Primitive *tmp, *cur;

  free(df->path);

  cur = df->primitives;
  while (cur != NULL) {
    tmp = cur;
    cur = cur->next;
    free(tmp);
  }

  free(df);
  df = NULL;
}


/* ----- static funcs ----- */


static char *
strip_str(char *str)
{
  assert (str != NULL);

  char *p, *s, *stripped;
  int unused;

  stripped = (char *)calloc(strlen(str)+1, sizeof(char));

  if (stripped == NULL) {
    return NULL;
  }

  unused = 0;
  for (p = str, s = stripped; *p != '\0'; p++) {
    if (isspace(*p)) {  /* Yes, we don't care about whitespaces between */
      unused++;
    } else {
      *s = *p;
      s++;
    }
  }
  *s = '\0';

  if (realloc(stripped, strlen(str)+1 - unused) == NULL) {
    free(stripped);
    return NULL;
  }

  return stripped;
}


static struct Primitive * 
primitive_create(char *line)
{
  assert(line != NULL);

  struct Primitive *prm; 

  prm = (struct Primitive *)malloc(sizeof(struct Primitive));
  if (prm == NULL) {
    return NULL;
  }

  prm->data = NULL;
 
  if (strcmp(line, entity_line.string) == 0) {
    memcpy(prm->type, entity_line.string, strlen(entity_line.string)+1);
  } else if (strcmp(line, entity_spline.string) == 0) {
    memcpy(prm->type, entity_spline.string, strlen(entity_spline.string)+1);
  } else {
    free(prm);
    return NULL;
  }

  return prm;
}


static void
fill_primitive(struct Primitive *prm, char *line, FILE *fp)
{
  assert(prm != NULL);
  assert(line != NULL);
  assert(fp != NULL);

  if (prm->type == entity_line.string) {
    prm->data = get_line(line, fp);
  } else if (prm->type == entity_spline.string) {
    prm->data = get_spline(line, fp);
  } else {
    /* need to somehow inform a caller */
  }
}


static struct LineData *
get_line(char *line, FILE *fp)
{
  return NULL;
}


static struct SplineData *
get_spline(char *line, FILE *fp)
{
  return NULL;
}

/*
static struct CircleData
get_circle(char *, FILE *fp)
{

}
*/
