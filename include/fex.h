#ifndef FEX_H
#define FEX_H

#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Function pointer types for original functions */
typedef int (*orig_open_t)(const char *pathname, int flags, ...);
typedef int (*orig_openat_t)(int dirfd, const char *pathname, int flags, ...);
typedef int (*orig_close_t)(int fd);
typedef ssize_t (*orig_read_t)(int fd, void *buf, size_t count);
typedef FILE *(*orig_fopen_t)(const char *pathname, const char *mode);
typedef int (*orig_fclose_t)(FILE *stream);
typedef size_t (*orig_fread_t)(void *ptr, size_t size, size_t nmemb,
                               FILE *stream);
typedef int (*orig_fseek_t)(FILE *stream, long offset, int whence);
typedef long (*orig_ftell_t)(FILE *stream);
typedef off_t (*orig_lseek_t)(int fd, off_t offset, int whence);
typedef void (*orig_rewind_t)(FILE *stream);
typedef int (*orig_fgetpos_t)(FILE *stream, fpos_t *pos);
typedef int (*orig_fsetpos_t)(FILE *stream, const fpos_t *pos);
typedef int (*orig_fgetc_t)(FILE *stream);
typedef char *(*orig_fgets_t)(char *s, int size, FILE *stream);
typedef int (*orig_getc_t)(FILE *stream);
typedef int (*orig_ungetc_t)(int c, FILE *stream);
typedef int (*orig_feof_t)(FILE *stream);
typedef int (*orig_ferror_t)(FILE *stream);
typedef void (*orig_clearerr_t)(FILE *stream);
typedef int (*orig_fileno_t)(FILE *stream);
typedef int (*orig_stat_t)(const char *pathname, struct stat *statbuf);
typedef int (*orig_fstat_t)(int fd, struct stat *statbuf);
typedef int (*orig_fstatat_t)(int dirfd, const char *pathname,
                              struct stat *statbuf, int flags);

/* Utility functions */
void fex_init(void);
void fex_log(const char *format, ...);

#endif // FEX_H