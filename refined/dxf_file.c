
/* Functions to work with DxfFile */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dxf_file.h"

static char * strip_str(char *);
static int is_entity(char *);
static struct Primitive * fill_primitive(char *, FILE *);
static int destroy_primitive(struct Primitive *prm);

static struct LineData * get_line(FILE *);
static struct SplineData * get_spline(FILE *);

struct entity_line_t entity_line = {"LINE", "AcDbLine", "10", "20", "11", "21"};
struct entity_spline_t entity_spline = {"SPLINE", "AcDbSpline", "10", "20", "40", "72", "73"};
/*
struct entity_circle_t entity_circle = {"CIRCLE", "10", "20", "40"};
*/


/*
  dxf_file_open: creates a DxfFile structure

  path: path to .dxf file.
  Note, if we open a link, we will save path *in* link, not path *of* link.
*/
struct DxfFile *
dxf_file_open(char *path)
{
  assert(path != NULL);

  FILE *fp;
  char *line, *stripped;
  int in_entities;
  size_t line_length;
  ssize_t ch_read;
  struct Primitive *prm, *cur;
  struct DxfFile *df;

  /* TODO: check for links, we identificate Linear by path
           Gonna use realpath() (man 3 realpath)
   */

  fp = fopen(path, "r");
  if (fp == NULL) {
    return NULL;
  }

  df = (struct DxfFile *)calloc(1, sizeof(struct DxfFile));
  df->primitives = NULL;
  df->resolved_path = (char *)calloc(strlen(path)+1, sizeof(char));
  memcpy(df->resolved_path, path, strlen(path)+1);

  cur = NULL;
  prm = NULL;
  line = NULL;
  line_length = 0;
  in_entities = 0;

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    if (ferror(fp)) {
      free(line);
      dxf_file_close(df);
      return NULL;
    }

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

    if (in_entities && is_entity(stripped)) {
      prm = fill_primitive(stripped, fp);
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

/*
  dxf_file_close: frees the memory of DxfFile

  df: DxfFile struct
  Caller can't access df after this routine.
*/
void
dxf_file_close(struct DxfFile *df)
{
  assert(df != NULL);

  struct Primitive *tmp, *cur;

  free(df->resolved_path);

  cur = df->primitives;
  while (cur != NULL) {
    tmp = cur;
    cur = cur->next;
    destroy_primitive(tmp);
    free(tmp);
  }

  free(df);
  df = NULL;
}


/* ----- static funcs ----- */


/*
  strip_str: returns a string w/out whitespaces
  Note that you need to free return pointer when you don't need it anymore.

  str: string w/ whitespaces (\n, \t, \v, " ")
*/
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


/*
  is_entity: compares the line w/ known entity's strings

  line: string to compare
*/
static int 
is_entity(char *line)
{
  if (strcmp(line, entity_line.string) == 0) {
    return 1;

  } else if (strcmp(line, entity_spline.string) == 0) {
    return 1;

  } else {
    return 0;
  }
}


/*
  fill_primitive: abstraction layer to parser funcs

  prm: Primitive struct to fill
  fp: opened file to parse
*/
static struct Primitive *
fill_primitive(char *line, FILE *fp)
{
  assert(line != NULL);
  assert(fp != NULL);

  struct Primitive *prm; 

  prm = (struct Primitive *)malloc(sizeof(struct Primitive));
  if (prm == NULL) {
    return NULL;
  }

  if (strcmp(line, entity_line.string) == 0) {
    prm->type = (char *)calloc(strlen(entity_line.string)+1, sizeof(char));
    memcpy(prm->type, entity_line.string, strlen(entity_line.string)+1);
    prm->data = (void *)get_line(fp);
    return prm;

  } else if (strcmp(line, entity_spline.string) == 0) {
    prm->type = (char *)calloc(strlen(entity_spline.string)+1, sizeof(char));
    memcpy(prm->type, entity_spline.string, strlen(entity_spline.string)+1);
    prm->data = (void *)get_spline(fp);
    return prm;

  } else {
    free(prm);
    return NULL;
  }
}


/*
  primitive_destroy: frees Primitive memory

  prm: Primitive struct to free
  Caller can't use pointer after this routine.
*/
static int
destroy_primitive(struct Primitive *prm)
{
  assert(prm != NULL);

  if (strcmp(prm->type, entity_line.string) == 0) {
    /* Nothing to do here */
    return 1;

  } else if (strcmp(prm->type, entity_spline.string) == 0) {
    
    return 1;

  } else {
    return 0;
  }
}


/* ----- entities parsing funcs ----- */


/*
  get_line: returns LINE entity data
*/
static struct LineData *
get_line(FILE *fp)
{
  assert(fp != NULL);

  struct LineData *ld;
  char *line, *stripped;
  size_t line_length;
  ssize_t ch_read;
  int read_line;

  ld = (struct LineData *)calloc(1, sizeof(struct LineData));
  line = NULL;
  line_length = 0;

  read_line = 1;
  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    if (feof(fp) || ferror(fp)) {
      free(line);
      free(ld);
      return NULL;
    }

    stripped = strip_str(line);
    if (stripped == NULL) {
      free(line);
      free(ld);
      return NULL;
    }

    if (read_line && strcmp(stripped, NEW_ENTITY) == 0) {
      break;

    } else if (strcmp(stripped, HANDLE) == 0) {
      read_line = 0;

    } else if (strcmp(stripped, ENTITY_DATA) == 0) {
      read_line = 0;

    } else if (strcmp(stripped, DIVIDER) == 0) {
      read_line = 1;

    } else if (strcmp(stripped, entity_line.dbsect) == 0) {
      read_line = 1;

    } else if (read_line && strcmp(stripped, entity_line.x_begin) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
      stripped = strip_str(line);
      ld->begin.x = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ld);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.y_begin) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
      stripped = strip_str(line);
      ld->begin.y = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ld);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.x_end) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
      stripped = strip_str(line);
      ld->end.x = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ld);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.y_end) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
      stripped = strip_str(line);
      ld->end.y = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ld);
        return NULL;
      }
    }
    free(stripped);
  }
  free(line);

  return ld;
}

/*
  get_spline: returns SPLINE entity data
*/
static struct SplineData *
get_spline(FILE *fp)
{
  return NULL;
}
