/*
 * Default VFS backend: stdio filesystem.
 */

#include "swevfs.h"

#ifndef SE_NO_STDIO

#include <stdio.h>
#include <string.h>

#include "../sweph.h" /* for swed, AS_MAXCH, PATH_SEPARATOR, DIR_GLUE, BFILE_R_ACCESS */

typedef struct swe_stdio_handle {
  FILE *fp;
} swe_stdio_handle;

static swe_vfs_handle vfs_open_stdio(int ifno, const char *fname, const char *ephepath, char *serr)
{
  int np, i, j;
  FILE *fp = NULL;
  char fn[AS_MAXCH];
  char *cpos[20];
  char s[2 * AS_MAXCH];
  char s1[AS_MAXCH];

  /* Resolve search path exactly like the original swi_fopen(). */
  if (ephepath == NULL) {
    ephepath = "";
  }
  strncpy(s1, ephepath, AS_MAXCH - 1);
  s1[AS_MAXCH - 1] = '\0';
  np = swi_cutstr(s1, PATH_SEPARATOR, cpos, 20);

  *s = '\0';
  for (i = 0; i < np; i++) {
    strcpy(s, cpos[i]);
    if (strcmp(s, ".") == 0) {
      *s = '\0';
    } else {
      j = (int) strlen(s);
      if (*s != '\0' && *(s + j - 1) != *DIR_GLUE)
        strcat(s, DIR_GLUE);
    }
    if (strlen(s) + strlen(fname) < AS_MAXCH) {
      strcat(s, fname);
    } else {
      if (serr != NULL)
        sprintf(serr, "error: file path and name must be shorter than %d.", AS_MAXCH);
      return NULL;
    }
    strncpy(fn, s, AS_MAXCH - 1);
    fn[AS_MAXCH - 1] = '\0';

    fp = fopen(fn, BFILE_R_ACCESS);
    if (fp != NULL) {
      swe_stdio_handle *h = (swe_stdio_handle *) malloc(sizeof(swe_stdio_handle));
      if (h == NULL) {
        fclose(fp);
        if (serr != NULL) strcpy(serr, "out of memory");
        return NULL;
      }
      h->fp = fp;
      (void) ifno;
      return (swe_vfs_handle) h;
    }
  }

  if (serr != NULL) {
    snprintf(s, AS_MAXCH, "SwissEph file '%s' not found in PATH '%s'", fname, ephepath);
    s[AS_MAXCH - 1] = '\0';
    strcpy(serr, s);
  }
  return NULL;
}

static size_t vfs_read_at_stdio(swe_vfs_handle hh, void *dst, size_t size, size_t count, int32 offset, char *serr)
{
  swe_stdio_handle *h = (swe_stdio_handle *) hh;
  if (h == NULL || h->fp == NULL) {
    if (serr != NULL) strcpy(serr, "invalid handle");
    return 0;
  }
  if (fseek(h->fp, (long) offset, SEEK_SET) != 0) {
    if (serr != NULL) strcpy(serr, "seek failed");
    return 0;
  }
  return fread(dst, size, count, h->fp);
}

static void vfs_close_stdio(swe_vfs_handle hh)
{
  swe_stdio_handle *h = (swe_stdio_handle *) hh;
  if (h == NULL) {
    return;
  }
  if (h->fp != NULL) {
    fclose(h->fp);
  }
  free(h);
}

static const swe_vfs_api g_stdio_api = {
  vfs_open_stdio,
  vfs_read_at_stdio,
  vfs_close_stdio,
};

const swe_vfs_api *swi_get_default_vfs_api(void)
{
  return &g_stdio_api;
}

#else

/*
 * No-stdio build: no default backend.
 * Host must call swe_set_vfs_api().
 */
const swe_vfs_api *swi_get_default_vfs_api(void)
{
  return NULL;
}

#endif

