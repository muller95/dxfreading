
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
static void get_sections(char **lines, size_t lines_quant, struct Sections *sections);
static void file_lines_destroy(char **lines, size_t line_quant);

static char * strip_str(char *str);

static union Entity * entities_read(char **base, size_t *entities_quant);
static union Entity * line_read(char **cur, size_t *offset);
static union Entity * spline_read(char **cur, size_t *offset);


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
  struct Sections sections;
  struct DxfFile *df;

  resolved_path = realpath(path, NULL);

  if (resolved_path == NULL) {
    resolved_path = strdup(path);
    if (resolved_path == NULL) {
      fprintf(stderr, "%s(): strdup() failure: %s\n", __func__, strerror(errno));
      return NULL;
    }
  }

  if ((fp = fopen(resolved_path, "r")) == NULL) {
    fprintf(stderr, "%s(): fopen(%s, \"r\") failure: %s\n", __func__, resolved_path, strerror(errno));
    return NULL;
  }

  df = (struct DxfFile *)calloc(1, sizeof(struct DxfFile));
  if (df == NULL) {
    fprintf(stderr, "%s(): calloc() failure: %s\n", __func__, strerror(errno));
    return NULL;
  }

  df->resolved_path = strdup(resolved_path);
  if (df->resolved_path == NULL) {
    fprintf(stderr, "%s(): strdup() failure: %s\n", __func__, strerror(errno));
    return NULL;
  }
  
  free(resolved_path);

  df->header = NULL;
  df->classes = NULL;
  df->tables = NULL;
  df->blocks = NULL;
  df->entities = NULL;
  df->objects = NULL;
  df->thumbnail = NULL;

  lines = file_lines_get(fp, &lines_quant);
  
  get_sections(lines, lines_quant, &sections);

/* df->header = header_read(lines, sections.header); */
/* df->classes = classes_read(lines, sections.classes); */
/* df->tables = tables_read(sections.tables); */
/* df->blocks = blocks_read(sections.blocks); */
  df->entities = entities_read(sections.entities, &(df->entities_quant));
/* df->objects = objects_read(sections.objects); */
/* df->thumbnail = thumbnail_read(sections.thumbnail); */

  if (df->entities == NULL) {
    fprintf(stderr, "%s(): entities_read() failure\n", __func__);
    file_lines_destroy(lines, lines_quant);
    return NULL;
  }

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
      fprintf(stderr, "%s(): file error: %s\n", __func__, strerror(errno));
      free(line);
      file_lines_destroy(lines, lq);
      return NULL;
    }

    stripped = strip_str(line);
    if (stripped == NULL) {
      fprintf(stderr, "%s(): strip_str() failure\n", __func__);
      free(line);
      file_lines_destroy(lines, lq);
      return NULL;
    }

    if (lq >= arr_size) {
      arr_size *= 2;
      lines = (char **)realloc(lines, arr_size * sizeof(char *));
      if (lines == NULL) {
        fprintf(stderr, "%s(): realloc() failure: %s\n", __func__, strerror(errno));
        free(line);
        free(stripped);
        file_lines_destroy(lines, lq);
        return NULL;
      }
    }

    lines[lq] = strdup(stripped);
    if (lines[lq] == NULL) {
      fprintf(stderr, "%s: strdup() failure\n", __func__);
      free(line);
      free(stripped);
      file_lines_destroy(lines, lq);
      return NULL;
    }

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


static void
get_sections(char **lines, size_t lines_quant, struct Sections *sections)
{
  char *line;
  size_t line_count;

  sections->header = NULL;
  sections->classes = NULL;
  sections->tables = NULL;
  sections->blocks = NULL;
  sections->entities = NULL;
  sections->objects = NULL;
  sections->thumbnail = NULL;

  for (line_count=0; line_count < lines_quant; line_count++) {
    line = lines[line_count];

    if (strcmp(line, "SECTION") == 0) {

      if (strcmp(lines[line_count+2], "HEADER") == 0) {
        sections->header = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "CLASSES") == 0) {
        sections->classes = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "TABLES") == 0) {
        sections->tables = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "BLOCKS") == 0) {
        sections->blocks = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "ENTITIES") == 0) {
        sections->entities = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "OBJECTS") == 0) {
        sections->objects = &lines[line_count+2];

      } else if (strcmp(lines[line_count+2], "THUMBNAIL") == 0) {
        sections->thumbnail = &lines[line_count+2];
      }
    }
  }
}


static union Entity * 
entities_read(char **base, size_t *entities_quant)
{
  char **cur;
  size_t eq, arr_size, offset;
  union Entity *entities, *ent;

  arr_size = ARR_SIZE;
  entities = (union Entity *)calloc(arr_size, sizeof(union Entity));

  eq = 0;
  cur = base;
  while (strcmp(*cur, "ENDSEC") != 0) {

    if (eq >= arr_size) {
      arr_size *= 2;
      entities = (union Entity *)realloc(entities, arr_size * sizeof(union Entity));
    }

    if (strcmp(*cur, entity_line.string) == 0) {
      ent = line_read(cur, &offset);
      memcpy(entities+eq, ent, sizeof(union Entity));
      free(ent);

      cur += offset;
      eq++;

    } else if (strcmp(*cur, entity_spline.string) == 0) {
      ent = spline_read(cur, &offset);
      memcpy(entities+eq, ent, sizeof(union Entity));
      free(ent);

      cur += offset;
      eq++;

    } else {
      cur++;
    }
  }

  if (eq == 0) {
    free(entities);
    return NULL;
  }

  entities = (union Entity *)realloc(entities, eq * sizeof(union Entity));
  *entities_quant = eq;

  return entities;
}


/* ----- entities parsing funcs ----- */


static union Entity *
line_read(char **cur, size_t *offset) 
{
  union Entity *ent;
  size_t ofs;

  ent = (union Entity *)calloc(1, sizeof(union Entity));

  ent->type = strdup(entity_line.string);

  ofs = 1;
  while (strcmp(*cur, entity_line.dbsect) != 0) {  /* We don't need some data yet */
    cur++;
    ofs++;
  }

  while (strcmp(*cur, OBJ_TYPE) != 0) {
    if (strcmp(*cur, entity_line.x_begin) == 0) {
      ent->line.begin.x = atof(*(++cur));
      ofs++;

    } else if (strcmp(*cur, entity_line.y_begin) == 0) {
      ent->line.begin.y = atof(*(++cur)); 
      ofs++;

    } else if (strcmp(*cur, entity_line.x_end) == 0) {
      ent->line.end.x = atof(*(++cur));
      ofs++;

    } else if (strcmp(*cur, entity_line.y_end) == 0) {
      ent->line.end.y = atof(*(++cur)); 
      ofs++;
    }
    cur++;
    ofs++;
  }

  *offset = ofs;
  return ent;
}


static union Entity *
spline_read(char **cur, size_t *offset) 
{
  union Entity *ent;
  struct Point *point;
  size_t ofs;

  ent = (union Entity *)calloc(1, sizeof(union Entity));

  ent->type = strdup(entity_spline.string);

  ent->spline.points = NULL;
  ofs = 0;
  while (strcmp(*cur, entity_spline.dbsect) != 0) { /* We don't need some data yet */
    cur++;
    ofs++;
  }

  while (strcmp(*cur, OBJ_TYPE) != 0) {
    if (strcmp(*cur, entity_spline.cps_quant) == 0) {
      ent->spline.cps_quant = (size_t)atoi(*(++cur));
      ent->spline.points = (struct Point *)calloc(ent->spline.cps_quant, sizeof(struct Point));
      point = ent->spline.points;
      ofs++;

    } else if (strcmp(*cur, entity_spline.cp_x) == 0) {
      if (point == NULL) {
        fprintf(stderr, "%s(): read failure\n", __func__);
        free(ent);
        return NULL;
      }

      point->x = atof(*(++cur));
      ofs++;
    
    } else if (strcmp(*cur, entity_spline.cp_y) == 0) {
      if (point == NULL) {
        fprintf(stderr, "%s(): read failure\n", __func__);
        free(ent);
        return NULL;
      }

      point->y = atof(*(++cur));
      ofs++;
      point++;

    } else if (strcmp(*cur, entity_spline.fitp_quant) == 0) {
      cur++;
    } else if (strcmp(*cur, entity_spline.knots_quant) == 0) {
      cur++;
    }
    cur++;
    ofs++;
  }

  *offset = ofs;
  return ent;
}
