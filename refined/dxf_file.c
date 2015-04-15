
/* Functions to work with DxfFile */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dxf_file.h"

static char * strip_str(char *);
static int is_entity(char *);
static struct Entity * entity_fill(char *, FILE *);
static int entity_destroy(struct Entity *ent);

static union EntityData * ent_line_parse(FILE *);
static union EntityData * ent_spline_parse(FILE *);

struct entity_line_t entity_line = {"LINE", "AcDbLine", "10", "20", "11", "21"};
struct entity_spline_t entity_spline = {"SPLINE", "AcDbSpline", "10", "20", "73"};


/*
  dxf_file_open: creates a DxfFile structure

  path: path to .dxf file.
  Note, if we open a link, we will save path _in_ link, not path _of_ link.
*/
struct DxfFile *
dxf_file_open(char *path)
{
  assert(path != NULL);

  FILE *fp;
  char *line, *stripped, *resolved_path;
  int in_entities;
  size_t line_length;
  ssize_t ch_read;
  struct Entity *ent, *cur;
  struct DxfFile *df;

  resolved_path = realpath(path, NULL);

  if (resolved_path == NULL) {
    resolved_path = (char *)calloc(strlen(path)+1, sizeof(char));
    memcpy(resolved_path, path, strlen(path)+1);
  }

  if ((fp = fopen(resolved_path, "r")) == NULL) {
    return NULL;
  }

  df = (struct DxfFile *)calloc(1, sizeof(struct DxfFile));
  df->entities = NULL;
  df->resolved_path = (char *)calloc(strlen(resolved_path)+1, sizeof(char));
  memcpy(df->resolved_path, resolved_path, strlen(resolved_path)+1);
  free(resolved_path);

  cur = NULL;
  ent = NULL;
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
      ent = entity_fill(stripped, fp);
      if (ent == NULL) {
        return NULL;
      }

      if (df->entities == NULL) {
        df->entities = ent;
        cur = ent;

      } else {
        cur->next = ent;
        cur = ent;
      }
      df->entities_quant++;
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

  struct Entity *tmp, *cur;

  free(df->resolved_path);

  cur = df->entities;
  while (cur != NULL) {
    tmp = cur;
    cur = cur->next;
    entity_destroy(tmp);
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
  entity_fill: abstraction layer to parser funcs

  line: current file line w/ entity type
  fp: opened file to parse
*/
static struct Entity *
entity_fill(char *line, FILE *fp)
{
  assert(line != NULL);
  assert(fp != NULL);

  struct Entity *ent; 

  ent = (struct Entity *)calloc(1, sizeof(struct Entity));
  if (ent == NULL) {
    return NULL;
  }

  if (strcmp(line, entity_line.string) == 0) {
    ent->data = ent_line_parse(fp);
    return ent;

  } else if (strcmp(line, entity_spline.string) == 0) {
    ent->data = ent_spline_parse(fp);
    return ent;

  } else {
    free(ent);
    return NULL;
  }
}


/*
  entity_destroy: frees Entity memory

  ent: Entity struct to free
  Caller can't use pointer after this routine.
*/
static int
entity_destroy(struct Entity *ent)
{
  assert(ent != NULL);

  if (strcmp(ent->data->type, entity_line.string) == 0) {
    free(ent->data->type);

    free(ent->data);
    return 1;

  } else if (strcmp(ent->data->type, entity_spline.string) == 0) {
    free(ent->data->type);

    free(ent->data->spline.points);
    
    free(ent->data);
    return 1;

  } else {
    return 0;
  }
}


/* ----- entities parsing funcs ----- */


/*
  ent_line_parse: returns LINE entity data
*/
static union EntityData *
ent_line_parse(FILE *fp)
{
  assert(fp != NULL);

  union EntityData *ed;
  char *line, *stripped;
  size_t line_length;
  ssize_t ch_read;
  int read_line;

  ed = (union EntityData *)calloc(1, sizeof(union EntityData));
  if (ed == NULL) {
    return NULL;
  }

  ed->line.type = strdup(entity_line.string);
  if (ed->line.type == NULL) {
    return NULL;
  }

  line = NULL;
  line_length = 0;

  read_line = 1;
  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    if (feof(fp) || ferror(fp)) {
      free(line);
      free(ed);
      return NULL;
    }

    stripped = strip_str(line);
    if (stripped == NULL) {
      free(line);
      free(ed);
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
        ed->line.begin.x = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.y_begin) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);
        ed->line.begin.y = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.x_end) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);
        ed->line.end.x = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_line.y_end) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);
        ed->line.end.y = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }
    }
    free(stripped);
  }
  free(line);

  return ed;
}

/*
  ent_spline_parse: returns SPLINE entity data
*/
static union EntityData *
ent_spline_parse(FILE *fp)
{
  assert(fp != NULL);

  union EntityData *ed;
  char *line, *stripped;
  size_t line_length;
  ssize_t ch_read;
  int read_line;
  struct Point *cur;

  cur = NULL;

  ed = (union EntityData *)calloc(1, sizeof(union EntityData));
  if (ed == NULL) {
    return NULL;
  }

  ed->spline.type = strdup(entity_spline.string);
  if (ed->spline.type == NULL) {
    return NULL;
  }

  line = NULL;
  line_length = 0;

  read_line = 1;
  while ((ch_read = getline(&line, &line_length, fp)) != -1) {
    if (feof(fp) || ferror(fp)) {
      free(line);
      free(ed);
      return NULL;
    }

    stripped = strip_str(line);
    if (stripped == NULL) {
      free(line);
      free(ed);
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

    } else if (strcmp(stripped, entity_spline.dbsect) == 0) {
      read_line = 1;

    } else if (read_line && strcmp(stripped, entity_spline.cp_x) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);

        cur->x = atof(stripped);

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_spline.cp_y) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);

        cur->y = atof(stripped);

        cur++;

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }

    } else if (read_line && strcmp(stripped, entity_spline.cps_quant) == 0) {
      free(stripped);
      if ((ch_read = getline(&line, &line_length, fp)) != -1) {
        stripped = strip_str(line);
        ed->spline.cps_quant = atoi(stripped);
        ed->spline.points = (struct Point *)calloc(ed->spline.cps_quant, sizeof(struct Point));
        cur = ed->spline.points;

      } else {
        free(stripped);
        free(line);
        free(ed);
        return NULL;
      }
  } else if (read_line && strcmp(stripped, "74") == 0) { 
    if ((ch_read = getline(&line, &line_length, fp)) != -1) {
      ;
    }
  }
    free(stripped);
  }
  free(line);

  return ed;
}
