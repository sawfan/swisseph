/*
 * Minimal VFS/callback layer for Swiss Ephemeris.
 */

#include "swevfs.h"

/*
 * Default backend is implemented in swevfs_stdio.c (native builds).
 * For targets where stdio/FS are not desired, the default is NULL.
 */
extern const swe_vfs_api *swi_get_default_vfs_api(void);

static const swe_vfs_api *g_vfs_api = NULL;

void CALL_CONV swe_set_vfs_api(const swe_vfs_api *api)
{
  if (api == NULL) {
    g_vfs_api = swi_get_default_vfs_api();
  } else {
    g_vfs_api = api;
  }
}

const swe_vfs_api *swi_get_vfs_api(void)
{
  if (g_vfs_api == NULL) {
    g_vfs_api = swi_get_default_vfs_api();
  }
  return g_vfs_api;
}

