/*
 * Minimal VFS/callback layer for Swiss Ephemeris.
 *
 * Goal: allow building for targets without a traditional filesystem
 * (e.g. wasm32-unknown-unknown) by letting the host provide ephemeris
 * file bytes via callbacks.
 *
 * This header intentionally keeps the surface small and independent from
 * stdio types like FILE*.
 */
#ifndef SWEVFS_H
#define SWEVFS_H

#include "../sweodef.h"

/*
 * swephexp.h defines CALL_CONV for the public Swiss Ephemeris API.
 * Keep swevfs.h usable as a standalone header by providing a fallback.
 */
#ifndef CALL_CONV
#define CALL_CONV
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A handle returned by the VFS backend. For an in-memory backend this might
 * be an index/pointer; for a filesystem backend it could wrap a FILE*.
 */
typedef void *swe_vfs_handle;

typedef struct swe_vfs_api {
  /*
   * Open a resource for reading.
   *
   * - fname is the requested file name (e.g. "sepl_18.se1").
   * - ephepath is the ephemeris path as configured by swe_set_ephe_path().
   *
   * Return a non-NULL handle on success, NULL on failure.
   *
   * If serr is non-NULL, a human readable error message may be written.
   */
  swe_vfs_handle (*open)(int ifno, const char *fname, const char *ephepath, char *serr);

  /*
   * Read exactly like pread(): read up to (size * count) bytes at absolute
   * byte offset from the start of the resource.
   *
   * Returns number of *items* successfully read (not bytes), like fread().
   *
   * This function must be safe for arbitrary offsets.
   */
  size_t (*read_at)(swe_vfs_handle h, void *dst, size_t size, size_t count, int32 offset, char *serr);

  /*
   * Close a handle returned by open().
   */
  void (*close)(swe_vfs_handle h);
} swe_vfs_api;

/*
 * Register a VFS implementation. Passing NULL resets to the default backend.
 */
void CALL_CONV swe_set_vfs_api(const struct swe_vfs_api *api);

/* Internal helpers used inside the C library. */
const swe_vfs_api *swi_get_vfs_api(void);

#ifdef __cplusplus
}
#endif

#endif /* SWEVFS_H */

