#define _GNU_SOURCE
#include "fex.h"
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Global variables for original function pointers */
static orig_open_t orig_open = NULL;
static orig_openat_t orig_openat = NULL;
static orig_close_t orig_close = NULL;
static orig_read_t orig_read = NULL;
static orig_fopen_t orig_fopen = NULL;
static orig_fclose_t orig_fclose = NULL;
static orig_fread_t orig_fread = NULL;
static orig_fseek_t orig_fseek = NULL;
static orig_ftell_t orig_ftell = NULL;
static orig_lseek_t orig_lseek = NULL;
static orig_rewind_t orig_rewind = NULL;
static orig_fgetpos_t orig_fgetpos = NULL;
static orig_fsetpos_t orig_fsetpos = NULL;
static orig_fgetc_t orig_fgetc = NULL;
static orig_fgets_t orig_fgets = NULL;
static orig_getc_t orig_getc = NULL;
static orig_ungetc_t orig_ungetc = NULL;
static orig_feof_t orig_feof = NULL;
static orig_ferror_t orig_ferror = NULL;
static orig_clearerr_t orig_clearerr = NULL;
static orig_fileno_t orig_fileno = NULL;
static orig_stat_t orig_stat = NULL;
static orig_fstat_t orig_fstat = NULL;
static orig_fstatat_t orig_fstatat = NULL;

/* Debug logging flag */
static int debug_enabled = 0;

/* Initialization function */
void fex_init(void) {
  static int initialized = 0;
  if (initialized)
    return;

  /* Check if debug logging is enabled */
  debug_enabled = getenv("FEX_DEBUG") != NULL;

  /* Load original function pointers */
  orig_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
  orig_openat = (orig_openat_t)dlsym(RTLD_NEXT, "openat");
  orig_close = (orig_close_t)dlsym(RTLD_NEXT, "close");
  orig_read = (orig_read_t)dlsym(RTLD_NEXT, "read");
  orig_fopen = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
  orig_fclose = (orig_fclose_t)dlsym(RTLD_NEXT, "fclose");
  orig_fread = (orig_fread_t)dlsym(RTLD_NEXT, "fread");
  orig_fseek = (orig_fseek_t)dlsym(RTLD_NEXT, "fseek");
  orig_ftell = (orig_ftell_t)dlsym(RTLD_NEXT, "ftell");
  orig_lseek = (orig_lseek_t)dlsym(RTLD_NEXT, "lseek");
  orig_rewind = (orig_rewind_t)dlsym(RTLD_NEXT, "rewind");
  orig_fgetpos = (orig_fgetpos_t)dlsym(RTLD_NEXT, "fgetpos");
  orig_fsetpos = (orig_fsetpos_t)dlsym(RTLD_NEXT, "fsetpos");
  orig_fgetc = (orig_fgetc_t)dlsym(RTLD_NEXT, "fgetc");
  orig_fgets = (orig_fgets_t)dlsym(RTLD_NEXT, "fgets");
  orig_getc = (orig_getc_t)dlsym(RTLD_NEXT, "getc");
  orig_ungetc = (orig_ungetc_t)dlsym(RTLD_NEXT, "ungetc");
  orig_feof = (orig_feof_t)dlsym(RTLD_NEXT, "feof");
  orig_ferror = (orig_ferror_t)dlsym(RTLD_NEXT, "ferror");
  orig_clearerr = (orig_clearerr_t)dlsym(RTLD_NEXT, "clearerr");
  orig_fileno = (orig_fileno_t)dlsym(RTLD_NEXT, "fileno");
  orig_stat = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
  orig_fstat = (orig_fstat_t)dlsym(RTLD_NEXT, "fstat");
  orig_fstatat = (orig_fstatat_t)dlsym(RTLD_NEXT, "fstatat");

  initialized = 1;
  fex_log("FEX library initialized\n");
}

/* Debug logging function */
void fex_log(const char *format, ...) {
  if (!debug_enabled)
    return;

  va_list args;
  va_start(args, format);
  fprintf(stderr, "[FEX] ");
  vfprintf(stderr, format, args);
  va_end(args);
}

/* Constructor - called when library is loaded */
__attribute__((constructor)) void fex_constructor(void) { fex_init(); }

/* ========== FILE DESCRIPTOR FUNCTIONS ========== */

int open(const char *pathname, int flags, ...) {
  fex_init();

  mode_t mode = 0;
  if (flags & O_CREAT) {
    va_list args;
    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);
  }

  fex_log("open(%s, %d, %o)\n", pathname, flags, mode);

  int result = orig_open(pathname, flags, mode);
  fex_log("open() returned %d\n", result);
  return result;
}

int openat(int dirfd, const char *pathname, int flags, ...) {
  fex_init();

  mode_t mode = 0;
  if (flags & O_CREAT) {
    va_list args;
    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);
  }

  fex_log("openat(%d, %s, %d, %o)\n", dirfd, pathname, flags, mode);

  int result = orig_openat(dirfd, pathname, flags, mode);
  fex_log("openat() returned %d\n", result);
  return result;
}

int close(int fd) {
  fex_init();
  fex_log("close(%d)\n", fd);

  int result = orig_close(fd);
  fex_log("close() returned %d\n", result);
  return result;
}

ssize_t read(int fd, void *buf, size_t count) {
  fex_init();
  fex_log("read(%d, %p, %zu)\n", fd, buf, count);

  ssize_t result = orig_read(fd, buf, count);
  fex_log("read() returned %zd\n", result);
  return result;
}

off_t lseek(int fd, off_t offset, int whence) {
  fex_init();
  fex_log("lseek(%d, %ld, %d)\n", fd, offset, whence);

  off_t result = orig_lseek(fd, offset, whence);
  fex_log("lseek() returned %ld\n", result);
  return result;
}

/* ========== FILE STREAM FUNCTIONS ========== */

FILE *fopen(const char *pathname, const char *mode) {
  fex_init();
  fex_log("fopen(%s, %s)\n", pathname, mode);

  FILE *result = orig_fopen(pathname, mode);
  fex_log("fopen() returned %p\n", result);
  return result;
}

int fclose(FILE *stream) {
  fex_init();
  fex_log("fclose(%p)\n", stream);

  int result = orig_fclose(stream);
  fex_log("fclose() returned %d\n", result);
  return result;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  fex_init();
  fex_log("fread(%p, %zu, %zu, %p)\n", ptr, size, nmemb, stream);

  size_t result = orig_fread(ptr, size, nmemb, stream);
  fex_log("fread() returned %zu\n", result);
  return result;
}

int fseek(FILE *stream, long offset, int whence) {
  fex_init();
  fex_log("fseek(%p, %ld, %d)\n", stream, offset, whence);

  int result = orig_fseek(stream, offset, whence);
  fex_log("fseek() returned %d\n", result);
  return result;
}

long ftell(FILE *stream) {
  fex_init();
  fex_log("ftell(%p)\n", stream);

  long result = orig_ftell(stream);
  fex_log("ftell() returned %ld\n", result);
  return result;
}

void rewind(FILE *stream) {
  fex_init();
  fex_log("rewind(%p)\n", stream);

  orig_rewind(stream);
  fex_log("rewind() completed\n");
}

int fgetpos(FILE *stream, fpos_t *pos) {
  fex_init();
  fex_log("fgetpos(%p, %p)\n", stream, pos);

  int result = orig_fgetpos(stream, pos);
  fex_log("fgetpos() returned %d\n", result);
  return result;
}

int fsetpos(FILE *stream, const fpos_t *pos) {
  fex_init();
  fex_log("fsetpos(%p, %p)\n", stream, pos);

  int result = orig_fsetpos(stream, pos);
  fex_log("fsetpos() returned %d\n", result);
  return result;
}

/* ========== CHARACTER I/O FUNCTIONS ========== */

int fgetc(FILE *stream) {
  fex_init();
  fex_log("fgetc(%p)\n", stream);

  int result = orig_fgetc(stream);
  fex_log("fgetc() returned %d\n", result);
  return result;
}

char *fgets(char *s, int size, FILE *stream) {
  fex_init();
  fex_log("fgets(%p, %d, %p)\n", s, size, stream);

  char *result = orig_fgets(s, size, stream);
  fex_log("fgets() returned %p\n", result);
  return result;
}

int getc(FILE *stream) {
  fex_init();
  fex_log("getc(%p)\n", stream);

  int result = orig_getc(stream);
  fex_log("getc() returned %d\n", result);
  return result;
}

int ungetc(int c, FILE *stream) {
  fex_init();
  fex_log("ungetc(%d, %p)\n", c, stream);

  int result = orig_ungetc(c, stream);
  fex_log("ungetc() returned %d\n", result);
  return result;
}

/* ========== FILE STATUS FUNCTIONS ========== */

int feof(FILE *stream) {
  fex_init();
  fex_log("feof(%p)\n", stream);

  int result = orig_feof(stream);
  fex_log("feof() returned %d\n", result);
  return result;
}

int ferror(FILE *stream) {
  fex_init();
  fex_log("ferror(%p)\n", stream);

  int result = orig_ferror(stream);
  fex_log("ferror() returned %d\n", result);
  return result;
}

void clearerr(FILE *stream) {
  fex_init();
  fex_log("clearerr(%p)\n", stream);

  orig_clearerr(stream);
  fex_log("clearerr() completed\n");
}

int fileno(FILE *stream) {
  fex_init();
  fex_log("fileno(%p)\n", stream);

  int result = orig_fileno(stream);
  fex_log("fileno() returned %d\n", result);
  return result;
}

/* ========== FILE STAT FUNCTIONS ========== */

int stat(const char *pathname, struct stat *statbuf) {
  fex_init();
  fex_log("stat(%s, %p)\n", pathname, statbuf);

  int result = orig_stat(pathname, statbuf);
  fex_log("stat() returned %d\n", result);
  return result;
}

int fstat(int fd, struct stat *statbuf) {
  fex_init();
  fex_log("fstat(%d, %p)\n", fd, statbuf);

  int result = orig_fstat(fd, statbuf);
  fex_log("fstat() returned %d\n", result);
  return result;
}

int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
  fex_init();
  fex_log("fstatat(%d, %s, %p, %d)\n", dirfd, pathname, statbuf, flags);

  int result = orig_fstatat(dirfd, pathname, statbuf, flags);
  fex_log("fstatat() returned %d\n", result);
  return result;
}