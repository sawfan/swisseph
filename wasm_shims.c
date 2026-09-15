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

#define WASM_SEEK_SET 0
#define WASM_SEEK_CUR 1
#define WASM_SEEK_END 2

typedef struct wasm_file_stub FILE;

extern size_t swisseph_vfs_file_len(const char *name);
extern size_t swisseph_vfs_read_file(const char *name, void *dst, size_t len, size_t offset);

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

      int zero_pad = 0;
      int width = 0;
      if (*p == '0') {
        zero_pad = 1;
        p++;
      }
      while (*p >= '0' && *p <= '9') {
        width = width * 10 + (*p - '0');
        p++;
      }

      /* Length modifiers used by Swiss Ephemeris diagnostics. */
      int long_arg = 0;
      if (*p == 'l') {
        long_arg = 1;
        p++;
      }

      if (*p == 's') {
        const char *s = va_arg(ap, const char *);
        if (s == NULL)
          s = "";
        size_t n = strlen(s);
        size_t pad = width > (int)n ? (size_t)(width - (int)n) : 0;
        while (pad-- > 0) {
          *out++ = ' ';
        }
        memcpy(out, s, n);
        out += n;
        p++;
        continue;
      }
      if (*p == 'd') {
        long lv = long_arg ? va_arg(ap, long) : (long)va_arg(ap, int);
        char tmp[32];
        int neg = 0;
        unsigned long u;
        if (lv < 0) {
          neg = 1;
          u = (unsigned long)(-lv);
        } else {
          u = (unsigned long)lv;
        }
        int i = 0;
        do {
          tmp[i++] = (char)('0' + (u % 10));
          u /= 10;
        } while (u && i < (int)sizeof(tmp));
        int digits = i;
        int sign_width = neg ? 1 : 0;
        char pad_char = zero_pad ? '0' : ' ';
        int pad = width - digits - sign_width;
        if (neg && zero_pad) {
          *out++ = '-';
          neg = 0;
        }
        while (pad-- > 0) {
          *out++ = pad_char;
        }
        if (neg) {
          *out++ = '-';
        }
        while (i > 0) {
          *out++ = tmp[--i];
        }
        p++;
        continue;
      }

      /* Unknown specifier: write the original marker literally enough for diagnostics. */
      *out++ = '%';
      if (zero_pad) {
        *out++ = '0';
      }
      if (width > 0) {
        char tmp[16];
        int i = 0;
        int w = width;
        do {
          tmp[i++] = (char)('0' + (w % 10));
          w /= 10;
        } while (w && i < (int)sizeof(tmp));
        while (i > 0) {
          *out++ = tmp[--i];
        }
      }
      if (long_arg) {
        *out++ = 'l';
      }
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
 * Minimal stdio shims backed by the Rust-uploaded in-memory ephemeris VFS.
 */
struct wasm_file_stub {
  char name[256];
  size_t offset;
  size_t len;
};

static FILE wasm_open_files[16];
static int wasm_open_file_used[16];

static const char *wasm_basename(const char *path) {
  const char *last = path;
  for (const char *p = path; *p; p++) {
    if (*p == '/' || *p == '\\') {
      last = p + 1;
    }
  }
  return last;
}

FILE *fopen(const char *restrict pathname, const char *restrict mode) {
  (void)mode;
  if (pathname == NULL) {
    return (FILE *)0;
  }

  const char *name = wasm_basename(pathname);
  size_t file_len = swisseph_vfs_file_len(name);
  if (file_len == 0) {
    return (FILE *)0;
  }

  for (int i = 0; i < 16; i++) {
    if (!wasm_open_file_used[i]) {
      wasm_open_file_used[i] = 1;
      strncpy(wasm_open_files[i].name, name, sizeof(wasm_open_files[i].name) - 1);
      wasm_open_files[i].name[sizeof(wasm_open_files[i].name) - 1] = 0;
      wasm_open_files[i].offset = 0;
      wasm_open_files[i].len = file_len;
      return &wasm_open_files[i];
    }
  }

  return (FILE *)0;
}

int fclose(void *_stream) {
  if (_stream == NULL) {
    return 0;
  }
  FILE *stream = (FILE *)_stream;
  for (int i = 0; i < 16; i++) {
    if (&wasm_open_files[i] == stream) {
      wasm_open_file_used[i] = 0;
      wasm_open_files[i].name[0] = 0;
      wasm_open_files[i].offset = 0;
      wasm_open_files[i].len = 0;
      break;
    }
  }
  return 0;
}

char *fgets(char *s, int size, void *_stream) {
  if (s == NULL || _stream == NULL || size <= 0) {
    return (char *)0;
  }

  FILE *stream = (FILE *)_stream;
  int written = 0;
  while (written < size - 1 && stream->offset < stream->len) {
    char ch = 0;
    size_t n = swisseph_vfs_read_file(stream->name, &ch, 1, stream->offset);
    if (n != 1) {
      break;
    }
    stream->offset += 1;
    s[written++] = ch;
    if (ch == '\n') {
      break;
    }
  }

  if (written == 0) {
    return (char *)0;
  }
  s[written] = 0;
  return s;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (ptr == NULL || stream == NULL || size == 0 || nmemb == 0) {
    return 0;
  }

  size_t want = size * nmemb;
  if (stream->offset > stream->len) {
    return 0;
  }
  size_t available = stream->len - stream->offset;
  size_t read_len = want < available ? want : available;
  size_t actual = swisseph_vfs_read_file(stream->name, ptr, read_len, stream->offset);
  stream->offset += actual;
  return actual / size;
}

int fseek(FILE *stream, long offset, int whence) {
  if (stream == NULL) {
    return -1;
  }

  long base;
  if (whence == WASM_SEEK_SET) {
    base = 0;
  } else if (whence == WASM_SEEK_CUR) {
    base = (long)stream->offset;
  } else if (whence == WASM_SEEK_END) {
    base = (long)stream->len;
  } else {
    return -1;
  }

  long next = base + offset;
  if (next < 0 || (size_t)next > stream->len) {
    return -1;
  }
  stream->offset = (size_t)next;
  return 0;
}

long ftell(FILE *stream) {
  if (stream == NULL) {
    return -1;
  }
  return (long)stream->offset;
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

