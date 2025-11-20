#ifndef FEX_H
#define FEX_H

#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* File tracking structure for .fex files */
typedef struct fex_file_entry {
  int fd;                   /* File descriptor */
  FILE *fp;                 /* FILE pointer (if opened with fopen) */
  char *original_filename;  /* Original filename */
  off_t original_size;      /* Original file size */
  char *header_string;      /* Generated C header string */
  char *footer_string;      /* Generated C footer string */
  off_t simulated_position; /* Current simulated position for seek/tell */
  off_t simulated_size;     /* Calculated simulated file size */
  off_t header_len;         /* Length of header string */
  off_t data_len;           /* Length of data section (6 * original_size) */
  off_t footer_start;       /* Starting position of footer section */
  char *buffer;             /* Pre-loaded buffer for efficient reading */
  size_t block_size;        /* Block size for buffer operations */
  off_t current_block;      /* Current block number being accessed */
  FILE *original_fp; /* File pointer to original file for buffer operations */
  struct fex_file_entry *next; /* Next entry in linked list */
} fex_file_entry_t;

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

/* File tracking functions */
int is_fex_file(const char *pathname);
char *resolve_pathname_at(int dirfd, const char *pathname);
char *generate_c_variable_name(const char *filename);
int generate_fex_code_data(const char *filename, off_t original_size,
                           char **header_string, char **footer_string,
                           off_t *simulated_size, off_t *header_len,
                           off_t *data_len, off_t *footer_start);
off_t get_real_file_position(fex_file_entry_t *entry, off_t simulated_position);
int get_simulated_character(fex_file_entry_t *entry, off_t position);
int load_block_into_buffer(fex_file_entry_t *entry, off_t block_number);
void initialize_fex_buffer(fex_file_entry_t *entry);
void free_fex_buffer(fex_file_entry_t *entry);
void track_fex_file_fd(int fd, const char *pathname, int flags);
void track_fex_file_fp(FILE *fp, const char *pathname, const char *mode);
void untrack_fex_file_fd(int fd);
void untrack_fex_file_fp(FILE *fp);
void print_fex_files_status(void);
#endif // FEX_H