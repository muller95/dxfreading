
/* Functions to work with DxfFile */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dxf_file.h"

#define ARR_SIZE 128

static char ** file_lines_get(FILE *fp, size_t *lines_quant);
static void file_lines_destroy(char **lines, size_t line_quant);

static char * strip_str(char *str);

static union Entity * entities_read(char **lines, size_t line_quant);
/*
  dxf_file_open: creates a DxfFile structure

  path: path to .dxf file.
  Note, if we open a link, we will save path _in_ link, not path _of_ link.
*/
struct DxfFile *
dxf_file_open(const char *path)
{
  assert(path != NULL);

  FILE *fp;
  char *resolved_path, **lines;
  size_t lines_quant;
  struct DxfFile *df;

  resolved_path = realpath(path, NULL);

  if (resolved_path == NULL) {
    resolved_path = strdup(path);
    if (resolved_path == NULL) {
      fprintf(stderr, "%s: can't strdup(): %s\n", __func__, strerror(errno));
      return NULL;
    }
  }

  if ((fp = fopen(resolved_path, "r")) == NULL) {
    fprintf(stderr, "%s: can't open %s: %s\n", __func__, resolved_path, strerror(errno));
    return NULL;
  }

  df = (struct DxfFile *)calloc(1, sizeof(struct DxfFile));
  df->resolved_path = strdup(resolved_path);
  
  free(resolved_path);

  df->header = NULL;
  df->classes = NULL;
  df->tables = NULL;
  df->blocks = NULL;
  df->entities = NULL;
  df->objects = NULL;
  df->thumbnail = NULL;

  lines = file_lines_get(fp, &lines_quant);

/* df->header = header_read(lines, lines_quant); */
/* df->classes = classes_read(lines, lines_quant); */
/* df->tables = tables_read(lines, lines_quant); */
/* df->blocks = blocks_read(lines, lines_quant); */
  df->entities = entities_read(lines, lines_quant);
/* df->objects = objects_read(lines, lines_quant); */
/* df->thumbnail = thumbnail_read(lines, lines_quant); */

/*  if (df->entities == NULL) {
    fprintf(stderr, "%s: entities_read() failure\n", __func__);
    file_lines_destroy(lines, lines_quant);
    return NULL
  } */

  file_lines_destroy(lines, lines_quant);
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

  free(df->resolved_path);

  if (df->header != NULL) {
    free(df->header);
  }

  if (df->classes != NULL) {
    free(df->classes);
  }

  if (df->tables != NULL) {
    free(df->tables);
  }

  if (df->blocks != NULL) {
    free(df->blocks);
  }

  if (df->entities != NULL) {
    free(df->entities);
  }

  if (df->objects != NULL) {
    free(df->objects);
  }

  if (df->thumbnail != NULL) {
    free(df->thumbnail);
  }

  free(df);
  df = NULL;
}


/* ----- static funcs ----- */


/*
  file_lines_get: return array of file lines

  fp: pointer to open file
  lines_quant: pointer to write lines quantity
*/
static char **
file_lines_get(FILE *fp, size_t *lines_quant)
{
  assert(fp != NULL);
  assert(lines_quant != NULL);

  char *line, *stripped, **lines;
  size_t line_length, arr_size, lq;
  ssize_t ch_read;

  line = NULL;
  line_length = 0;

  arr_size = (size_t)ARR_SIZE;
  lines = (char **)calloc(arr_size, sizeof(char *));

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    if (ferror(fp)) {
      fprintf(stderr, "%s: file error: %s\n", __func__, strerror(errno));
      free(line);
      file_lines_destroy(lines, lq);
      return NULL;
    }

    stripped = strip_str(line);
    if (stripped == NULL) {
      fprintf(stderr, "%s: strip_str() failure\n", __func__);
      free(line);
      file_lines_destroy(lines, lq);
      return NULL;
    }

    if (lq >= arr_size) {
      arr_size *= 2;
      lines = (char **)realloc(lines, arr_size * sizeof(char *));
      if (lines == NULL) {
        fprintf(stderr, "%s: realloc() failure: %s\n", __func__, strerror(errno));
        free(line);
        free(stripped);
        return NULL;
      }
    }

    lines[lq] = strdup(stripped);

    free(line);
    free(stripped);
    line = NULL;
    lq++;
  }

  lines = (char **)realloc(lines, lq * sizeof(char *));
  *lines_quant = lq;

  return lines;
}


/*
  file_lines_destroy: frees a memory of file lines list

  lines: pointer to lines array
  quant: quantity of lines in array
  Caller can't access pointer after this routine.
*/
static void
file_lines_destroy(char *lines[], size_t lines_quant)
{
  assert(lines != NULL);
  assert(lines_quant >= 0);

  size_t i;

  for (i=0; i < lines_quant; i++) {
    free(lines[i]);
  }

  free(lines);
  lines = NULL;
}


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

static union Entity * 
entities_read(char **lines, size_t lines_quant)
{
  return NULL;
}


/* ----- entities parsing funcs ----- */


