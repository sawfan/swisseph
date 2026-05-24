// Shim header for building Swiss Ephemeris C sources with SE_NO_JPL/NO_JPL.
//
// Some Swiss Ephemeris versions reference JPL constants/functions even when
// NO_JPL is defined, relying on those symbols being available via headers.
// When we compile in constrained environments (e.g. wasm32-unknown-unknown)
// we explicitly build with -DSE_NO_JPL -DNO_JPL, and those references must be
// guarded or stubbed.
//
// We force-include this header for wasm builds via `.cargo/config.toml` so we
// don't need to patch the vendored upstream C sources.

#pragma once

#if defined(SE_NO_JPL) || defined(NO_JPL)

// JPL target IDs. Values are not used in NO_JPL builds; they just need to exist
// so compilation succeeds when upstream sources reference them.
#ifndef J_EARTH
#define J_EARTH 0
#endif
#ifndef J_MOON
#define J_MOON 0
#endif
#ifndef J_MERCURY
#define J_MERCURY 0
#endif
#ifndef J_VENUS
#define J_VENUS 0
#endif
#ifndef J_MARS
#define J_MARS 0
#endif
#ifndef J_JUPITER
#define J_JUPITER 0
#endif
#ifndef J_SATURN
#define J_SATURN 0
#endif
#ifndef J_URANUS
#define J_URANUS 0
#endif
#ifndef J_NEPTUNE
#define J_NEPTUNE 0
#endif
#ifndef J_PLUTO
#define J_PLUTO 0
#endif
#ifndef J_SUN
#define J_SUN 0
#endif
#ifndef J_SBARY
#define J_SBARY 0
#endif

// JPL-related internal helpers.
//
// In NO_JPL builds these should never be called. We provide minimal stubs so
// compilation succeeds even if the upstream sources missed a preprocessor
// guard.
static inline void swi_close_jpl_file(void) {}

static inline int swi_pleph(double tjd, int ipli, int ipl, double *x, char *serr) {
  (void)tjd;
  (void)ipli;
  (void)ipl;
  (void)x;
  (void)serr;
  return -1;
}

static inline int swi_open_jpl_file(
    void *swed_p,
    const char *fname,
    const char *fpath,
    char *serr
) {
  (void)swed_p;
  (void)fname;
  (void)fpath;
  (void)serr;
  return -1;
}

static inline int swi_get_jpl_denum(void) {
  return 0;
}

#endif

