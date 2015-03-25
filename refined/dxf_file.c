#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common_structs.h"

#define ENTITIES "ENTITIES\r\n"
#define ENDSECS "ENDSECS\r\n"

static int max_points;

static int 
new_figure(char *line, struct DxfFile *df, struct DxfPrimitive **cur)
{
	int res, type;

	res = 0;
	if (strcmp(line, "LINE\r\n") == 0) {
		type = DXF_TYPE_LINE;
		res = 1;
	}
	else if (strcmp(line, "SPLINE\r\n") == 0) {
		type = DXF_TYPE_SPLINE;
		res = 1;
	}

	if (res == 1) {
		struct DxfPrimitive *dp, *prev;	

		max_points = 64;
		dp = (struct DxfPrimitive *)malloc(sizeof(struct DxfPrimitive));
		dp->points = (struct Point *)malloc(sizeof(struct Point) * max_points);
		dp->type = type;

		if (*cur == NULL) {
			dp->next = NULL;
			*cur = dp;
			df->primitives = dp;
		}
		else {
			prev = *cur;
			prev->next = dp;
			*cur = dp;
		}
	}

	return res;
}

void
dxf_file_create(char *path, struct DxfFile *df, int quant)
{

  assert(path != NULL);
  assert(df != NULL);
  assert(quant > 0);

  FILE *fp;
  char *line;
  int in_entities, n, count;
	size_t line_length;
  ssize_t ch_read;
  struct DxfPrimitive *dp;

  fp = fopen(path, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  dp = NULL;
  df->primitives = NULL;
  df->quant = quant;
  df->path = (char *)calloc(strlen(path)+1, sizeof(char));
  memcpy(df->path, path, strlen(path)+1);

 // line = NULL;
  line_length = 0;
  in_entities = 0;
  n = 0;	
	count = 0;

  while ((ch_read = getline(&line, &line_length, fp)) != -1) {

    if (strcmp(line, ENTITIES) == 0) {
      in_entities = 1;
    } else if (strcmp(line, ENDSECS) == 0) {
      in_entities = 0;
	  	break;
    }

    if (in_entities) {
			if (new_figure(line, df, &dp)) {
				n = 0;
			} else if (dp == NULL)
				continue;

			if (strcmp(line, " 10\r\n") == 0) {
				count++;
				getline(&line, &line_length, fp);
				dp->points[n].x = atof(line);
			}
			if (strcmp(line, " 20\r\n") == 0) {
				count++;
				getline(&line, &line_length, fp);
				dp->points[n].y = atof(line);
			}
			if (count == 2) {
				n++;
				dp->npoints = n;
				if (n == max_points) {
					max_points *= 2;
					dp->points = (struct Point *)realloc(dp->points, sizeof(struct Point) * max_points);
				}
				count = 0;
			}
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
