/*
 * Minimal libc-style shims for wasm32-unknown-unknown builds.
 *
 * This file is compiled into the Swiss Ephemeris static library when targeting
 * `wasm32-unknown-unknown`.
 *
 * We only provide functionality that the demo requires. In particular, we
 * implement a tiny `sprintf()` that supports `%s`, `%d`, and `%%`.
 *
 * Notes:
 * - The implementation is intentionally minimal and not a full libc.
 * - `sprintf()` is variadic, which is difficult to implement from Rust on
 *   stable. Providing it in C ensures the signature matches the C call sites.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int sprintf(char *dst, const char *fmt, ...) {
  if (dst == NULL || fmt == NULL) {
    return 0;
  }

  va_list ap;
  va_start(ap, fmt);

  const char *p = fmt;
  char *out = dst;

  while (*p) {
    if (*p == '%') {
      p++;
      if (*p == '%') {
        *out++ = '%';
        p++;
        continue;
      }
      if (*p == 's') {
        const char *s = va_arg(ap, const char *);
        if (s == NULL)
          s = "";
        size_t n = strlen(s);
        memcpy(out, s, n);
        out += n;
        p++;
        continue;
      }
      if (*p == 'd') {
        int v = va_arg(ap, int);
        char tmp[32];
        int neg = 0;
        unsigned int u;
        if (v < 0) {
          neg = 1;
          u = (unsigned int)(-v);
        } else {
          u = (unsigned int)v;
        }
        int i = 0;
        do {
          tmp[i++] = (char)('0' + (u % 10));
          u /= 10;
        } while (u && i < (int)sizeof(tmp));
        if (neg && i < (int)sizeof(tmp)) {
          tmp[i++] = '-';
        }
        while (i > 0) {
          *out++ = tmp[--i];
        }
        p++;
        continue;
      }

      /* Unknown specifier: just write '%' and the specifier literally. */
      *out++ = '%';
      if (*p) {
        *out++ = *p++;
      }
      continue;
    }

    *out++ = *p++;
  }

  *out = 0;

  va_end(ap);
  return (int)(out - dst);
}

/*
 * Minimal `toupper` implementation (ASCII only).
 *
 * Swiss Ephemeris uses this for parsing / case folding.
 */
int toupper(int c) {
  if (c >= 'a' && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

/*
 * Minimal `tolower` implementation (ASCII only).
 */
int tolower(int c) {
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  }
  return c;
}

/*
 * Minimal `strcpy` implementation.
 */
char *strcpy(char *dst, const char *src) {
  char *d = dst;
  while ((*d++ = *src++) != 0) {
    /* copy including null terminator */
  }
  return dst;
}

/*
 * Minimal `strcat` implementation.
 */
char *strcat(char *dst, const char *src) {
  char *d = dst;
  while (*d) {
    d++;
  }
  while ((*d++ = *src++) != 0) {
    /* append including null terminator */
  }
  return dst;
}

/*
 * Minimal `strchr` implementation.
 */
char *strchr(const char *s, int c) {
  const char ch = (char)c;
  while (*s) {
    if (*s == ch) {
      return (char *)s;
    }
    s++;
  }
  return ch == 0 ? (char *)s : (char *)0;
}

/*
 * Minimal `strncmp` implementation.
 */
int strncmp(const char *a, const char *b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    unsigned char ac = (unsigned char)a[i];
    unsigned char bc = (unsigned char)b[i];
    if (ac != bc) {
      return ac < bc ? -1 : 1;
    }
    if (ac == 0) {
      return 0;
    }
  }
  return 0;
}

/*
 * Minimal `strstr` implementation.
 */
char *strstr(const char *haystack, const char *needle) {
  if (!*needle) {
    return (char *)haystack;
  }

  for (const char *h = haystack; *h; h++) {
    const char *h2 = h;
    const char *n2 = needle;
    while (*h2 && *n2 && *h2 == *n2) {
      h2++;
      n2++;
    }
    if (!*n2) {
      return (char *)h;
    }
  }

  return (char *)0;
}

/*
 * Minimal `strcmp` implementation.
 */
int strcmp(const char *a, const char *b) {
  while (*a && (*a == *b)) {
    a++;
    b++;
  }
  return (unsigned char)*a - (unsigned char)*b;
}

/*
 * Minimal `strncpy` implementation.
 */
char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  for (; i < n && src[i] != 0; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {
    dst[i] = 0;
  }
  return dst;
}

/*
 * Minimal `strrchr` implementation.
 */
char *strrchr(const char *s, int c) {
  const char ch = (char)c;
  const char *last = (const char *)0;
  do {
    if (*s == ch) {
      last = s;
    }
  } while (*s++);
  return (char *)last;
}

/*
 * Minimal `strpbrk` implementation.
 */
char *strpbrk(const char *s, const char *accept) {
  for (const char *p = s; *p; p++) {
    for (const char *a = accept; *a; a++) {
      if (*p == *a) {
        return (char *)p;
      }
    }
  }

  return (char *)0;
}

/*
 * Minimal `isdigit` implementation (ASCII only).
 */
int isdigit(int c) {
  return (c >= '0' && c <= '9') ? 1 : 0;
}

/*
 * Minimal `atoi` implementation.
 */
int atoi(const char *nptr) {
  if (nptr == NULL) {
    return 0;
  }

  const char *p = nptr;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' ||
         *p == '\v') {
    p++;
  }

  int sign = 1;
  if (*p == '-') {
    sign = -1;
    p++;
  } else if (*p == '+') {
    p++;
  }

  int v = 0;
  while (isdigit((unsigned char)*p)) {
    v = (v * 10) + (*p - '0');
    p++;
  }

  return v * sign;
}

/*
 * Minimal `atol` implementation.
 */
long atol(const char *nptr) {
  if (nptr == NULL) {
    return 0;
  }

  const char *p = nptr;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' ||
         *p == '\v') {
    p++;
  }

  long sign = 1;
  if (*p == '-') {
    sign = -1;
    p++;
  } else if (*p == '+') {
    p++;
  }

  long v = 0;
  while (isdigit((unsigned char)*p)) {
    v = (v * 10) + (*p - '0');
    p++;
  }

  return v * sign;
}

/*
 * Very small `atof` stub.
 *
 * Swiss Ephemeris uses this while parsing numeric fields from text files.
 * For wasm demos we only need a value that doesn't crash; returning 0.0 is
 * sufficient for the code paths we use.
 */
double atof(const char *_nptr) {
  (void)_nptr;
  return 0.0;
}

/*
 * Minimal `getenv` stub.
 */
char *getenv(const char *_name) {
  (void)_name;
  return (char *)0;
}

/*
 * Minimal `rewind` stub (no-op).
 */
void rewind(void *_stream) {
  (void)_stream;
}

/*
 * Minimal stdio stubs.
 *
 * With `SE_NO_STDIO` enabled these should not be used for real I/O, but some
 * units still reference them. We provide stubs to keep the linker happy.
 */
int fclose(void *_stream) {
  (void)_stream;
  return 0;
}

char *fgets(char *s, int size, void *_stream) {
  (void)s;
  (void)size;
  (void)_stream;
  return (char *)0;
}

/*
 * Additional stdio-like stubs.
 *
 * Some Swiss Ephemeris units still reference these even with `SE_NO_STDIO`.
 * These are no-op / EOF stubs intended only to satisfy the linker.
 */
typedef struct wasm_file_stub {
  int _unused;
} FILE;

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  (void)ptr;
  (void)size;
  (void)nmemb;
  (void)stream;
  return 0;
}

int fseek(FILE *stream, long offset, int whence) {
  (void)stream;
  (void)offset;
  (void)whence;
  return -1;
}

long ftell(FILE *stream) {
  (void)stream;
  return -1;
}

/*
 * Minimal bump allocator for `malloc`/`free`.
 *
 * This is NOT a general-purpose allocator; it only supports allocating and
 * never reclaims memory. This is sufficient for wasm demo code paths that
 * allocate once during initialization.
 */
static uint8_t wasm_heap[1024 * 1024];
static size_t wasm_heap_used = 0;

static size_t wasm_align_up(size_t x, size_t align) {
  return (x + (align - 1)) & ~(align - 1);
}

void *malloc(size_t size) {
  if (size == 0) {
    return (void *)0;
  }

  const size_t align = 8;
  size_t start = wasm_align_up(wasm_heap_used, align);
  if (start + size > sizeof(wasm_heap)) {
    return (void *)0;
  }

  void *p = (void *)&wasm_heap[start];
  wasm_heap_used = start + size;
  return p;
}

void free(void *_ptr) {
  (void)_ptr;
}

/*
 * Minimal `strdup` implementation.
 *
 * Backed by our bump allocator.
 */
char *strdup(const char *s) {
  size_t n = strlen(s);
  char *out = (char *)malloc(n + 1);
  if (out == NULL) {
    return (char *)0;
  }
  memcpy(out, s, n);
  out[n] = 0;
  return out;
}

