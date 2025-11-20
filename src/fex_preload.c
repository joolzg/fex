#define _GNU_SOURCE
#include "fex.h"
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define FEX_DEFAULT_BLOCK_SIZE 4096

static const char hex_table[256][6] = {
    "0x00, ", "0x01, ", "0x02, ", "0x03, ", "0x04, ", "0x05, ", "0x06, ",
    "0x07, ", "0x08, ", "0x09, ", "0x0a, ", "0x0b, ", "0x0c, ", "0x0d, ",
    "0x0e, ", "0x0f, ", "0x10, ", "0x11, ", "0x12, ", "0x13, ", "0x14, ",
    "0x15, ", "0x16, ", "0x17, ", "0x18, ", "0x19, ", "0x1a, ", "0x1b, ",
    "0x1c, ", "0x1d, ", "0x1e, ", "0x1f, ", "0x20, ", "0x21, ", "0x22, ",
    "0x23, ", "0x24, ", "0x25, ", "0x26, ", "0x27, ", "0x28, ", "0x29, ",
    "0x2a, ", "0x2b, ", "0x2c, ", "0x2d, ", "0x2e, ", "0x2f, ", "0x30, ",
    "0x31, ", "0x32, ", "0x33, ", "0x34, ", "0x35, ", "0x36, ", "0x37, ",
    "0x38, ", "0x39, ", "0x3a, ", "0x3b, ", "0x3c, ", "0x3d, ", "0x3e, ",
    "0x3f, ", "0x40, ", "0x41, ", "0x42, ", "0x43, ", "0x44, ", "0x45, ",
    "0x46, ", "0x47, ", "0x48, ", "0x49, ", "0x4a, ", "0x4b, ", "0x4c, ",
    "0x4d, ", "0x4e, ", "0x4f, ", "0x50, ", "0x51, ", "0x52, ", "0x53, ",
    "0x54, ", "0x55, ", "0x56, ", "0x57, ", "0x58, ", "0x59, ", "0x5a, ",
    "0x5b, ", "0x5c, ", "0x5d, ", "0x5e, ", "0x5f, ", "0x60, ", "0x61, ",
    "0x62, ", "0x63, ", "0x64, ", "0x65, ", "0x66, ", "0x67, ", "0x68, ",
    "0x69, ", "0x6a, ", "0x6b, ", "0x6c, ", "0x6d, ", "0x6e, ", "0x6f, ",
    "0x70, ", "0x71, ", "0x72, ", "0x73, ", "0x74, ", "0x75, ", "0x76, ",
    "0x77, ", "0x78, ", "0x79, ", "0x7a, ", "0x7b, ", "0x7c, ", "0x7d, ",
    "0x7e, ", "0x7f, ", "0x80, ", "0x81, ", "0x82, ", "0x83, ", "0x84, ",
    "0x85, ", "0x86, ", "0x87, ", "0x88, ", "0x89, ", "0x8a, ", "0x8b, ",
    "0x8c, ", "0x8d, ", "0x8e, ", "0x8f, ", "0x90, ", "0x91, ", "0x92, ",
    "0x93, ", "0x94, ", "0x95, ", "0x96, ", "0x97, ", "0x98, ", "0x99, ",
    "0x9a, ", "0x9b, ", "0x9c, ", "0x9d, ", "0x9e, ", "0x9f, ", "0xa0, ",
    "0xa1, ", "0xa2, ", "0xa3, ", "0xa4, ", "0xa5, ", "0xa6, ", "0xa7, ",
    "0xa8, ", "0xa9, ", "0xaa, ", "0xab, ", "0xac, ", "0xad, ", "0xae, ",
    "0xaf, ", "0xb0, ", "0xb1, ", "0xb2, ", "0xb3, ", "0xb4, ", "0xb5, ",
    "0xb6, ", "0xb7, ", "0xb8, ", "0xb9, ", "0xba, ", "0xbb, ", "0xbc, ",
    "0xbd, ", "0xbe, ", "0xbf, ", "0xc0, ", "0xc1, ", "0xc2, ", "0xc3, ",
    "0xc4, ", "0xc5, ", "0xc6, ", "0xc7, ", "0xc8, ", "0xc9, ", "0xca, ",
    "0xcb, ", "0xcc, ", "0xcd, ", "0xce, ", "0xcf, ", "0xd0, ", "0xd1, ",
    "0xd2, ", "0xd3, ", "0xd4, ", "0xd5, ", "0xd6, ", "0xd7, ", "0xd8, ",
    "0xd9, ", "0xda, ", "0xdb, ", "0xdc, ", "0xdd, ", "0xde, ", "0xdf, ",
    "0xe0, ", "0xe1, ", "0xe2, ", "0xe3, ", "0xe4, ", "0xe5, ", "0xe6, ",
    "0xe7, ", "0xe8, ", "0xe9, ", "0xea, ", "0xeb, ", "0xec, ", "0xed, ",
    "0xee, ", "0xef, ", "0xf0, ", "0xf1, ", "0xf2, ", "0xf3, ", "0xf4, ",
    "0xf5, ", "0xf6, ", "0xf7, ", "0xf8, ", "0xf9, ", "0xfa, ", "0xfb, ",
    "0xfc, ", "0xfd, ", "0xfe, ", "0xff, "};

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

/* .fex file tracking */
static fex_file_entry_t *fex_files_head = NULL;
static pthread_mutex_t fex_files_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Initialization function */
void fex_init(void) {
  static int initialized = 0;
  if (initialized)
    return;

  /* Check if debug logging is enabled */
  debug_enabled = getenv("FEX_DEBUG") != NULL;

  /* Check if status should be printed on exit */
  if (getenv("FEX_SHOW_STATUS")) {
    atexit(print_fex_files_status);
  }

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

/* Get configurable block size from environment variable */
size_t get_fex_block_size(void) {
  const char *block_size_str = getenv("FEX_BLOCK_SIZE");
  if (block_size_str) {
    char *endptr;
    unsigned long size = strtoul(block_size_str, &endptr, 10);

    /* Validate the input */
    if (*endptr == '\0' && size >= 1024 && size <= 1024 * 1024) {
      /* Valid size between 1KB and 1MB */
      fex_log("Using custom block size: %lu bytes\n", size);
      return (size_t)size;
    } else {
      fex_log("Invalid FEX_BLOCK_SIZE value '%s', using default %d bytes\n",
              block_size_str, FEX_DEFAULT_BLOCK_SIZE);
    }
  }

  return FEX_DEFAULT_BLOCK_SIZE;
}

/* ========== .FEX FILE TRACKING FUNCTIONS ========== */

/* Check if a file has .fex extension */
int is_fex_file(const char *pathname) {
  if (!pathname)
    return 0;

  const char *ext = strrchr(pathname, '.');
  return (ext && strcmp(ext, ".fex") == 0);
}

/* Resolve full pathname for openat/fstatat operations */
char *resolve_pathname_at(int dirfd, const char *pathname) {
  if (!pathname)
    return NULL;

  /* If pathname is absolute or dirfd is AT_FDCWD, use pathname as-is */
  if (pathname[0] == '/' || dirfd == AT_FDCWD) {
    return strdup(pathname);
  }

  /* Get the directory path from dirfd */
  char dirpath[PATH_MAX];
  char fdpath[64];
  snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", dirfd);

  ssize_t len = readlink(fdpath, dirpath, sizeof(dirpath) - 1);
  if (len == -1) {
    /* If we can't resolve the directory, just use the relative pathname */
    fex_log("Failed to resolve directory for fd %d, using relative path\n",
            dirfd);
    return strdup(pathname);
  }
  dirpath[len] = '\0';

  /* Construct full pathname */
  size_t full_len =
      len + 1 + strlen(pathname) + 1; /* dir + '/' + file + '\0' */
  char *full_pathname = malloc(full_len);
  if (!full_pathname)
    return NULL;

  snprintf(full_pathname, full_len, "%s/%s", dirpath, pathname);
  return full_pathname;
}

/* Generate C variable name from filename (without extension) */
char *generate_c_variable_name(const char *filename) {
  if (!filename)
    return NULL;

  /* Find the base filename (after last slash) */
  const char *base = strrchr(filename, '/');
  if (base) {
    base++; /* Skip the slash */
  } else {
    base = filename;
  }

  /* Find the extension and calculate length without it */
  const char *ext = strrchr(base, '.');
  size_t len = ext ? (size_t)(ext - base) : strlen(base);

  /* Allocate memory for the variable name */
  char *var_name = malloc(len + 1);
  if (!var_name)
    return NULL;

  /* Copy and sanitize the name */
  for (size_t i = 0; i < len; i++) {
    char c = base[i];
    /* Replace invalid characters with underscores */
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '_') {
      var_name[i] = c;
    } else {
      var_name[i] = '_';
    }
  }
  var_name[len] = '\0';

  /* Ensure it doesn't start with a digit */
  if (len > 0 && var_name[0] >= '0' && var_name[0] <= '9') {
    /* Prepend underscore by shifting and adding */
    char *new_name = malloc(len + 2);
    if (new_name) {
      new_name[0] = '_';
      strcpy(new_name + 1, var_name);
      free(var_name);
      var_name = new_name;
    }
  }

  return var_name;
}

/* Generate C code strings and calculate simulated size for a .fex file */
int generate_fex_code_data(const char *filename, off_t original_size,
                           char **header_string, char **footer_string,
                           off_t *simulated_size, off_t *header_len,
                           off_t *data_len, off_t *footer_start) {
  if (!filename || !header_string || !footer_string || !simulated_size ||
      !header_len || !data_len || !footer_start) {
    return -1;
  }

  /* Initialize output parameters */
  *header_string = NULL;
  *footer_string = NULL;
  *simulated_size = 0;
  *header_len = 0;
  *data_len = 0;
  *footer_start = 0;

  /* Generate C variable name */
  char *var_name = generate_c_variable_name(filename);
  if (!var_name) {
    return -1;
  }

  /* Generate header string */
  size_t header_buf_len =
      strlen(var_name) + 50; /* Buffer for "unsigned char []= {" */
  *header_string = malloc(header_buf_len);
  if (!*header_string) {
    free(var_name);
    return -1;
  }
  snprintf(*header_string, header_buf_len, "unsigned char %s[] = {", var_name);

  /* Generate footer string */
  size_t footer_buf_len =
      strlen(var_name) +
      100; /* Buffer for "};\nunsigned long _SIZE = <number>;" */
  *footer_string = malloc(footer_buf_len);
  if (!*footer_string) {
    free(var_name);
    free(*header_string);
    *header_string = NULL;
    return -1;
  }
  snprintf(*footer_string, footer_buf_len, "};\nunsigned long %s_SIZE = %ld;",
           var_name, original_size);

  /* Calculate all size components */
  *header_len = strlen(*header_string);
  *data_len = sizeof(hex_table[0]) *
              original_size; /* 6 chars per byte in C array format */
  *footer_start = *header_len + *data_len;
  *simulated_size = *header_len + *data_len + strlen(*footer_string);

  fex_log("Generated C code for %s:\n", filename);
  fex_log("Header: %s\n", *header_string);
  fex_log("Footer: %s\n", *footer_string);
  fex_log("Simulated size: %ld bytes (header=%ld, data=%ld, footer=%ld)\n",
          *simulated_size, *header_len, *data_len, strlen(*footer_string));

  free(var_name);
  return 0;
}

/* Find a tracked .fex file by file descriptor */
fex_file_entry_t *find_fex_file_by_fd(int fd) {
  pthread_mutex_lock(&fex_files_mutex);
  fex_file_entry_t *current = fex_files_head;
  while (current) {
    if (current->fd == fd) {
      pthread_mutex_unlock(&fex_files_mutex);
      return current;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&fex_files_mutex);
  return NULL;
}

/* Find a tracked .fex file by FILE pointer */
fex_file_entry_t *find_fex_file_by_fp(FILE *fp) {
  pthread_mutex_lock(&fex_files_mutex);
  fex_file_entry_t *current = fex_files_head;
  while (current) {
    if (current->fp == fp) {
      pthread_mutex_unlock(&fex_files_mutex);
      return current;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&fex_files_mutex);
  return NULL;
}

/* Track a .fex file opened with file descriptor */
void track_fex_file_fd(int fd, const char *pathname, int flags) {
  if (!is_fex_file(pathname) || fd < 0)
    return;

  /* Don't track files opened for write operations */
  if (flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_APPEND)) {
    fex_log("Skipping .fex file tracking (opened for write): fd=%d, "
            "filename=%s, flags=0x%x\n",
            fd, pathname, flags);
    return;
  }

  pthread_mutex_lock(&fex_files_mutex);

  /* Get file size */
  struct stat st;
  off_t file_size = 0;
  if (orig_fstat(fd, &st) == 0) {
    file_size = st.st_size;
  }

  /* Create new entry */
  fex_file_entry_t *entry = malloc(sizeof(fex_file_entry_t));
  if (entry) {
    entry->fd = fd;
    entry->fp = NULL;
    entry->original_filename = strdup(pathname);
    entry->original_size = file_size;
    entry->simulated_position = 0;

    /* Generate C code strings and calculate simulated size */
    if (generate_fex_code_data(pathname, file_size, &entry->header_string,
                               &entry->footer_string, &entry->simulated_size,
                               &entry->header_len, &entry->data_len,
                               &entry->footer_start) != 0) {
      /* Failed to generate code data */
      entry->header_string = NULL;
      entry->footer_string = NULL;
      entry->simulated_size = 0;
      entry->header_len = 0;
      entry->data_len = 0;
      entry->footer_start = 0;
    }

    /* Initialize buffer fields */
    entry->buffer = NULL;
    entry->block_size = 0;
    entry->current_block = 0;
    entry->original_fp = NULL;

    /* Initialize buffer with simulated content */
    initialize_fex_buffer(entry);

    entry->next = fex_files_head;
    fex_files_head = entry;

    fex_log("Tracking .fex file: fd=%d, filename=%s, size=%ld\n", fd, pathname,
            file_size);
  }

  pthread_mutex_unlock(&fex_files_mutex);
}

/* Track a .fex file opened with FILE pointer */
void track_fex_file_fp(FILE *fp, const char *pathname, const char *mode) {
  if (!is_fex_file(pathname) || !fp)
    return;

  /* Don't track files opened for write operations */
  if (mode && (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+') ||
               strstr(mode, "r+"))) {
    fex_log("Skipping .fex file tracking (opened for write): fp=%p, "
            "filename=%s, mode=%s\n",
            fp, pathname, mode);
    return;
  }

  pthread_mutex_lock(&fex_files_mutex);

  /* Get file size via fileno */
  int fd = orig_fileno(fp);
  struct stat st;
  off_t file_size = 0;
  if (fd >= 0 && orig_fstat(fd, &st) == 0) {
    file_size = st.st_size;
  }

  /* Create new entry */
  fex_file_entry_t *entry = malloc(sizeof(fex_file_entry_t));
  if (entry) {
    entry->fd = fd;
    entry->fp = fp;
    entry->original_filename = strdup(pathname);
    entry->original_size = file_size;
    entry->simulated_position = 0;

    /* Generate C code strings and calculate simulated size */
    if (generate_fex_code_data(pathname, file_size, &entry->header_string,
                               &entry->footer_string, &entry->simulated_size,
                               &entry->header_len, &entry->data_len,
                               &entry->footer_start) != 0) {
      /* Failed to generate code data */
      entry->header_string = NULL;
      entry->footer_string = NULL;
      entry->simulated_size = 0;
      entry->header_len = 0;
      entry->data_len = 0;
      entry->footer_start = 0;
    }

    /* Initialize buffer fields */
    entry->buffer = NULL;
    entry->block_size = 0;
    entry->current_block = 0;
    entry->original_fp = NULL;

    /* Initialize buffer with simulated content */
    initialize_fex_buffer(entry);

    entry->next = fex_files_head;
    fex_files_head = entry;

    fex_log("Tracking .fex file: fp=%p, fd=%d, filename=%s, size=%ld\n", fp, fd,
            pathname, file_size);
  }

  pthread_mutex_unlock(&fex_files_mutex);
}

/* Remove tracking for a file descriptor */
void untrack_fex_file_fd(int fd) {
  pthread_mutex_lock(&fex_files_mutex);

  fex_file_entry_t **current = &fex_files_head;
  while (*current) {
    fex_file_entry_t *entry = *current;
    if (entry->fd == fd) {
      *current = entry->next;
      fex_log("Untracking .fex file: fd=%d, filename=%s\n", entry->fd,
              entry->original_filename);
      free_fex_buffer(entry);
      free(entry->original_filename);
      free(entry->header_string);
      free(entry->footer_string);
      free(entry);
      break;
    } else {
      current = &(entry->next);
    }
  }

  pthread_mutex_unlock(&fex_files_mutex);
}

/* Remove tracking for a FILE pointer */
void untrack_fex_file_fp(FILE *fp) {
  pthread_mutex_lock(&fex_files_mutex);

  fex_file_entry_t **current = &fex_files_head;
  while (*current) {
    fex_file_entry_t *entry = *current;
    if (entry->fp == fp) {
      *current = entry->next;
      fex_log("Untracking .fex file: fp=%p, filename=%s\n", entry->fp,
              entry->original_filename);
      free_fex_buffer(entry);
      free(entry->original_filename);
      free(entry->header_string);
      free(entry->footer_string);
      free(entry);
      break;
    } else {
      current = &(entry->next);
    }
  }

  pthread_mutex_unlock(&fex_files_mutex);
}

/* Print status of all tracked .fex files */
void print_fex_files_status(void) {
  pthread_mutex_lock(&fex_files_mutex);

  fex_log("=== Currently tracked .fex files ===\n");
  fex_file_entry_t *current = fex_files_head;
  int count = 0;

  while (current) {
    fex_log("  [%d] %s (fd=%d, fp=%p, original_size=%ld, simulated_size=%ld)\n",
            ++count, current->original_filename, current->fd, current->fp,
            current->original_size, current->simulated_size);
    fex_log("      Buffer: %p (block_size=%zu, current_block=%ld)\n",
            current->buffer, current->block_size, current->current_block);
    if (current->header_string) {
      fex_log("      Header: %s\n", current->header_string);
    }
    if (current->footer_string) {
      fex_log("      Footer: %s\n", current->footer_string);
    }
    current = current->next;
  }

  if (count == 0) {
    fex_log("  No .fex files currently tracked\n");
  }
  fex_log("=== End of .fex files status ===\n");

  pthread_mutex_unlock(&fex_files_mutex);
}

/* Load specific block into buffer */
int load_block_into_buffer(fex_file_entry_t *entry, off_t block_number) {
  if (!entry || !entry->original_fp || !entry->buffer) {
    return -1;
  }

  off_t block_start_pos = block_number * entry->block_size;

  /* Seek to the block position in the original file */
  if (orig_fseek(entry->original_fp, block_start_pos, SEEK_SET) != 0) {
    fex_log("Failed to seek to block %ld position %ld in original file %s\n",
            block_number, block_start_pos, entry->original_filename);
    return -1;
  }

  /* Read the block into buffer */
  size_t bytes_read =
      orig_fread(entry->buffer, 1, entry->block_size, entry->original_fp);

  /* Update tracking information */
  entry->current_block = block_number;

  fex_log("Loaded block %ld (%zu bytes) from position %ld for .fex file %s\n",
          block_number, bytes_read, block_start_pos, entry->original_filename);

  return 0;
}

/* Initialize buffer for .fex file entry */
void initialize_fex_buffer(fex_file_entry_t *entry) {
  if (!entry) {
    return;
  }

  /* Use a configurable block size (4KB default) */
  entry->block_size = get_fex_block_size();

  /* Allocate buffer */
  entry->buffer = malloc(entry->block_size);
  if (!entry->buffer) {
    fex_log("Failed to allocate buffer of size %zu for .fex file %s\n",
            entry->block_size, entry->original_filename);
    return;
  }

  /* Open original file for buffer operations */
  entry->original_fp = orig_fopen(entry->original_filename, "rb");
  if (!entry->original_fp) {
    fex_log("Failed to open original file %s for buffer operations\n",
            entry->original_filename);
  } else {
    fex_log("Keeping original file %s open for buffer operations\n",
            entry->original_filename);
  }
  entry->current_block = -1;

  fex_log("Initialized buffer for .fex file %s: block_size=%zu, "
          "current_block=%ld\n",
          entry->original_filename, entry->block_size, entry->current_block);
}

/* Free buffer for .fex file entry */
void free_fex_buffer(fex_file_entry_t *entry) {
  if (!entry) {
    return;
  }

  if (entry->buffer) {
    fex_log("Freeing buffer for .fex file %s\n", entry->original_filename);
    free(entry->buffer);
    entry->buffer = NULL;
  }

  /* Close the original file if it's open */
  if (entry->original_fp) {
    fex_log("Closing original file %s\n", entry->original_filename);
    orig_fclose(entry->original_fp);
    entry->original_fp = NULL;
  }

  entry->block_size = 0;
}

size_t read_bytes_from_buffer(fex_file_entry_t *entry, unsigned char *buffer,
                              size_t size) {
  if (!entry || !entry->header_string || !entry->footer_string) {
    return 0;
  }

  size_t bytes_read = 0;
  off_t start_position = entry->simulated_position;
  while (size) {
    size_t added = 1;

    /* Use pre-calculated values from the structure */
    if (entry->simulated_position < entry->header_len) {
      added = MIN(size, entry->header_len - entry->simulated_position);
      memcpy(buffer, entry->header_string + entry->simulated_position, added);
    } else if (entry->simulated_position < entry->footer_start) {
      /* Calculate the real position in the original file */
      off_t data_offset = entry->simulated_position - entry->header_len;
      off_t real_position = data_offset / sizeof(hex_table[0]);
      off_t block_number = real_position / entry->block_size;
      /* Load the block if it's not currently loaded */
      if (block_number != entry->current_block) {
        entry->current_block = block_number;
        if (load_block_into_buffer(entry, block_number) != 0) {
          fex_log("read_bytes_into_buffer() failed to load block %ld for .fex "
                  "file %s\n",
                  block_number, entry->original_filename);
        } else {
          fex_log("read_bytes_into_buffer() successfully loaded block %ld for "
                  ".fex file %s\n",
                  block_number, entry->original_filename);
        }
      }
      const char *hex_value =
          hex_table[entry->buffer[real_position % entry->block_size]];
      int which_char = data_offset % sizeof(hex_table[0]);
      added = MIN(size, sizeof(hex_table[0]) - which_char);
      memcpy(buffer, hex_value + which_char, added);
    } else {
      off_t pos = entry->simulated_position - entry->footer_start;
      size_t footer_len = strlen(entry->footer_string);
      if (pos >= footer_len) {
        added = 0; /* Beyond footer */
      } else {
        added = MIN(size, footer_len - pos);
        memcpy(buffer, entry->footer_string + pos, added);
      }
    }

    if (added == 0) {
      break; /* No more data to read */
    }

    buffer += added;
    entry->simulated_position += added;
    bytes_read += added;
    size -= added;
  }

  fex_log("fgetc() read at position %ld, %d bytes for .fex file %s\n",
          start_position, bytes_read, entry->original_filename);

  return bytes_read;
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

  /* Track .fex files */
  if (result >= 0) {
    track_fex_file_fd(result, pathname, flags);
  }

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

  /* Track .fex files */
  if (result >= 0) {
    /* Resolve full pathname for .fex detection and tracking */
    char *full_pathname = resolve_pathname_at(dirfd, pathname);
    if (full_pathname) {
      track_fex_file_fd(result, full_pathname, flags);
      free(full_pathname);
    }
  }

  return result;
}

int close(int fd) {
  fex_init();
  fex_log("close(%d)\n", fd);

  /* Untrack .fex files before closing */
  untrack_fex_file_fd(fd);

  int result = orig_close(fd);
  fex_log("close() returned %d\n", result);
  return result;
}

ssize_t read(int fd, void *buf, size_t count) {
  fex_init();
  fex_log("read(%d, %p, %zu)\n", fd, buf, count);

  fex_file_entry_t *entry = find_fex_file_by_fd(fd);
  if (entry) {
    size_t bytes_read = read_bytes_from_buffer(entry, buf, count);
    fex_log("read() simulated read %zu bytes (%zu elements) for .fex file %s\n",
            bytes_read, bytes_read, entry->original_filename);
    return bytes_read;
  }

  ssize_t result = orig_read(fd, buf, count);
  fex_log("read() returned %zd\n", result);
  return result;
}

off_t lseek(int fd, off_t offset, int whence) {
  fex_init();
  fex_log("lseek(%d, %ld, %d)\n", fd, offset, whence);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fd(fd);
  if (entry) {
    /* Always simulate for tracked .fex files */
    off_t simulated_size = entry->simulated_size;
    off_t new_position;

    if (whence == SEEK_END) {
      /* Seeking from end */
      new_position = simulated_size + offset;
    } else if (whence == SEEK_SET) {
      /* Seeking from beginning */
      new_position = offset;
    } else if (whence == SEEK_CUR) {
      /* Seeking from current position */
      new_position = entry->simulated_position + offset;
    } else {
      /* Invalid whence value */
      errno = EINVAL;
      return -1;
    }

    /* Validate the new position */
    if (new_position < 0) {
      fex_log("lseek() invalid position %ld (negative) for .fex file %s\n",
              new_position, entry->original_filename);
      errno = EINVAL;
      return -1;
    }

    /* Allow seeking beyond end - this is standard POSIX behavior */
    entry->simulated_position = new_position;

    fex_log("lseek() simulated seek (whence=%d): position=%ld for .fex file %s "
            "(simulated size: %ld)\n",
            whence, entry->simulated_position, entry->original_filename,
            simulated_size);
    return entry->simulated_position;
  }

  off_t result = orig_lseek(fd, offset, whence);
  fex_log("lseek() returned %ld\n", result);
  return result;
} /* ========== FILE STREAM FUNCTIONS ========== */

FILE *fopen(const char *pathname, const char *mode) {
  fex_init();
  fex_log("fopen(%s, %s)\n", pathname, mode);

  FILE *result = orig_fopen(pathname, mode);
  fex_log("fopen() returned %p\n", result);

  /* Track .fex files */
  if (result) {
    track_fex_file_fp(result, pathname, mode);
  }

  return result;
}

int fclose(FILE *stream) {
  fex_init();
  fex_log("fclose(%p)\n", stream);

  /* Untrack .fex files before closing */
  untrack_fex_file_fp(stream);

  int result = orig_fclose(stream);
  fex_log("fclose() returned %d\n", result);
  return result;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  fex_init();
  fex_log("fread(%p, %zu, %zu, %p)\n", ptr, size, nmemb, stream);

  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    size_t total_bytes = size * nmemb;
    size_t bytes_read = read_bytes_from_buffer(entry, ptr, total_bytes);
    size_t result = bytes_read / size;
    fex_log(
        "fread() simulated read %zu bytes (%zu elements) for .fex file %s\n",
        bytes_read, result, entry->original_filename);
    return result;
  }

  size_t result = orig_fread(ptr, size, nmemb, stream);
  fex_log("fread() returned %zu\n", result);
  return result;
}

int fseek(FILE *stream, long offset, int whence) {
  fex_init();
  fex_log("fseek(%p, %ld, %d)\n", stream, offset, whence);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* Always simulate for tracked .fex files */
    off_t simulated_size = entry->simulated_size;
    off_t new_position;

    if (whence == SEEK_END) {
      /* Seeking from end */
      new_position = simulated_size + offset;
    } else if (whence == SEEK_SET) {
      /* Seeking from beginning */
      new_position = offset;
    } else if (whence == SEEK_CUR) {
      /* Seeking from current position */
      new_position = entry->simulated_position + offset;
    } else {
      /* Invalid whence value */
      errno = EINVAL;
      return -1;
    }

    /* Validate the new position */
    if (new_position < 0) {
      fex_log("fseek() invalid position %ld (negative) for .fex file %s\n",
              new_position, entry->original_filename);
      errno = EINVAL;
      return -1;
    }

    /* Allow seeking beyond end - this is standard POSIX behavior */
    entry->simulated_position = new_position;

    fex_log("fseek() simulated seek (whence=%d): position=%ld for .fex file %s "
            "(simulated size: %ld)\n",
            whence, entry->simulated_position, entry->original_filename,
            simulated_size);
    return 0; /* Success */
  }

  int result = orig_fseek(stream, offset, whence);
  fex_log("fseek() returned %d\n", result);
  return result;
}
long ftell(FILE *stream) {
  fex_init();
  fex_log("ftell(%p)\n", stream);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    fex_log("ftell() returning simulated position %ld for .fex file %s\n",
            entry->simulated_position, entry->original_filename);
    return entry->simulated_position;
  }

  long result = orig_ftell(stream);
  fex_log("ftell() returned %ld\n", result);
  return result;
}

void rewind(FILE *stream) {
  fex_init();
  fex_log("rewind(%p)\n", stream);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    entry->simulated_position = 0;

    fex_log("rewind() reset simulated position to 0 and invalidated block for "
            ".fex file %s\n",
            entry->original_filename);
    return;
  }

  orig_rewind(stream);
  fex_log("rewind() completed\n");
}

int fgetpos(FILE *stream, fpos_t *pos) {
  fex_init();
  fex_log("fgetpos(%p, %p)\n", stream, pos);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* For .fex files, we need to get the original position and modify it */
    int result = orig_fgetpos(stream, pos);
    if (result == 0 && pos) {
      /* Store our simulated position in the fpos_t structure
       * Note: This is implementation-specific but works for most glibc systems
       */
      memcpy(pos, &entry->simulated_position, sizeof(off_t));
    }
    fex_log("fgetpos() returning simulated position %ld for .fex file %s\n",
            entry->simulated_position, entry->original_filename);
    return result;
  }

  int result = orig_fgetpos(stream, pos);
  fex_log("fgetpos() returned %d\n", result);
  return result;
}

int fsetpos(FILE *stream, const fpos_t *pos) {
  fex_init();
  fex_log("fsetpos(%p, %p)\n", stream, pos);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* For .fex files, extract the position from fpos_t and set our simulated
     * position */
    if (pos) {
      off_t new_position;
      memcpy(&new_position, pos, sizeof(off_t));

      /* Validate position is within simulated file bounds */
      off_t simulated_size = entry->simulated_size;
      if (new_position < 0) {
        fex_log("fsetpos() invalid position %ld (negative) for .fex file %s\n",
                new_position, entry->original_filename);
        errno = EINVAL;
        return -1; /* Error */
      }
      if (new_position > simulated_size) {
        fex_log("fsetpos() position %ld exceeds simulated size %ld for .fex "
                "file %s\n",
                new_position, simulated_size, entry->original_filename);
        /* Allow seeking beyond end - this is typically allowed in POSIX */
      }

      entry->simulated_position = new_position;

      fex_log("fsetpos() set simulated position to %ld for .fex file %s "
              "(simulated size: %ld)\n",
              entry->simulated_position, entry->original_filename,
              simulated_size);
    }
    return 0; /* Success */
  }

  int result = orig_fsetpos(stream, pos);
  fex_log("fsetpos() returned %d\n", result);
  return result;
}

int fgetc(FILE *stream) {
  fex_init();
  fex_log("fgetc(%p)\n", stream);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* For .fex files, check if we're reading from original content or simulated
     * format */
    if (entry->simulated_position >= entry->simulated_size) {
      /* At or beyond end of simulated file */
      fex_log("fgetc() at end of simulated file (pos=%ld, size=%ld) for .fex "
              "file %s\n",
              entry->simulated_position, entry->simulated_size,
              entry->original_filename);
      return EOF;
    }

    unsigned char the_char;
    size_t bytes_read = read_bytes_from_buffer(entry, &the_char, 1);

    if (bytes_read == 0) {
      /* No bytes read - end of file or error */
      fex_log("fgetc() no bytes read at position %ld for .fex file %s\n",
              entry->simulated_position, entry->original_filename);
      return EOF;
    }

    return (int)the_char;
  }

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

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    if (entry->simulated_position >= entry->simulated_size) {
      /* At or beyond end of simulated file */
      fex_log("getc() at end of simulated file (pos=%ld, size=%ld) for .fex "
              "file %s\n",
              entry->simulated_position, entry->simulated_size,
              entry->original_filename);
      return EOF;
    }

    /* Read character from original content buffer or simulated content */
    unsigned char character;
    size_t bytes_read = read_bytes_from_buffer(entry, &character, 1);

    return (int)character;
  }

  int result = orig_getc(stream);
  fex_log("getc() returned %d\n", result);

  return result;
}

int ungetc(int c, FILE *stream) {
  fex_init();
  fex_log("ungetc(%d, %p)\n", c, stream);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* For .fex files, we simulate ungetc by moving position back one place */
    if (entry->simulated_position > 0) {
      entry->simulated_position--;
      fex_log(
          "ungetc() moved simulated position back to %ld for .fex file %s\n",
          entry->simulated_position, entry->original_filename);
      return c; /* Return the character as if successfully pushed back */
    } else {
      /* Can't unget at position 0 */
      fex_log("ungetc() failed - already at position 0 for .fex file %s\n",
              entry->original_filename);
      return EOF;
    }
  }

  int result = orig_ungetc(c, stream);
  fex_log("ungetc() returned %d\n", result);
  return result;
}

/* ========== FILE STATUS FUNCTIONS ========== */

int feof(FILE *stream) {
  fex_init();
  fex_log("feof(%p)\n", stream);

  /* Check if this is a tracked .fex file */
  fex_file_entry_t *entry = find_fex_file_by_fp(stream);
  if (entry) {
    /* For .fex files, check if we're at or past the simulated file end */
    off_t simulated_size = entry->simulated_size;
    int is_eof = (entry->simulated_position >= simulated_size) ? 1 : 0;

    fex_log("feof() simulated check: position=%ld, size=%ld, EOF=%d for .fex "
            "file %s\n",
            entry->simulated_position, simulated_size, is_eof,
            entry->original_filename);

    return is_eof;
  }

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

  /* If this is a .fex file and stat succeeded, modify the size-related fields
   */
  if (result == 0 && is_fex_file(pathname) && statbuf) {
    /* Calculate simulated size for .fex file */
    off_t original_size = statbuf->st_size;
    char *header_string = NULL;
    char *footer_string = NULL;
    off_t simulated_size = 0;
    off_t header_len = 0;
    off_t data_len = 0;
    off_t footer_start = 0;

    if (generate_fex_code_data(pathname, original_size, &header_string,
                               &footer_string, &simulated_size, &header_len,
                               &data_len, &footer_start) == 0 &&
        simulated_size > 0) {
      /* Update stat buffer with simulated values */
      statbuf->st_size = simulated_size;
      statbuf->st_blksize = get_fex_block_size(); /* Configurable block size */
      statbuf->st_blocks = (simulated_size + (statbuf->st_blksize - 1)) /
                           statbuf->st_blksize; /* Round up to blocks */

      fex_log("stat() modified .fex file stats: original_size=%ld, "
              "simulated_size=%ld, blocks=%ld\n",
              original_size, simulated_size, statbuf->st_blocks);
    }

    free(header_string);
    free(footer_string);
  }

  fex_log("stat() returned %d\n", result);
  return result;
}

int fstat(int fd, struct stat *statbuf) {
  fex_init();
  fex_log("fstat(%d, %p)\n", fd, statbuf);

  int result = orig_fstat(fd, statbuf);

  /* Check if this file descriptor corresponds to a tracked .fex file */
  if (result == 0 && statbuf) {
    fex_file_entry_t *entry = find_fex_file_by_fd(fd);
    if (entry) {
      /* Update stat buffer with simulated values from tracked entry */
      statbuf->st_size = entry->simulated_size;
      statbuf->st_blksize = get_fex_block_size(); /* Configurable block size */
      statbuf->st_blocks = (entry->simulated_size + (statbuf->st_blksize - 1)) /
                           statbuf->st_blksize; /* Round up to blocks */

      fex_log("fstat() modified tracked .fex file stats: original_size=%ld, "
              "simulated_size=%ld, blocks=%ld\n",
              entry->original_size, entry->simulated_size, statbuf->st_blocks);
    }
  }

  fex_log("fstat() returned %d\n", result);
  return result;
}

int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
  fex_init();
  fex_log("fstatat(%d, %s, %p, %d)\n", dirfd, pathname, statbuf, flags);

  int result = orig_fstatat(dirfd, pathname, statbuf, flags);

  /* If this is a .fex file and stat succeeded, modify the size-related fields
   */
  if (result == 0 && statbuf) {
    /* Resolve full pathname for .fex detection */
    char *full_pathname = resolve_pathname_at(dirfd, pathname);
    if (full_pathname && is_fex_file(full_pathname)) {
      /* Calculate simulated size for .fex file */
      off_t original_size = statbuf->st_size;
      char *header_string = NULL;
      char *footer_string = NULL;
      off_t simulated_size = 0;
      off_t header_len = 0;
      off_t data_len = 0;
      off_t footer_start = 0;

      if (generate_fex_code_data(full_pathname, original_size, &header_string,
                                 &footer_string, &simulated_size, &header_len,
                                 &data_len, &footer_start) == 0 &&
          simulated_size > 0) {
        /* Update stat buffer with simulated values */
        statbuf->st_size = simulated_size;
        statbuf->st_blksize =
            get_fex_block_size(); /* Configurable block size */
        statbuf->st_blocks = (simulated_size + (statbuf->st_blksize - 1)) /
                             statbuf->st_blksize; /* Round up to blocks */

        fex_log("fstatat() modified .fex file stats: original_size=%ld, "
                "simulated_size=%ld, blocks=%ld\n",
                original_size, simulated_size, statbuf->st_blocks);
      }

      free(header_string);
      free(footer_string);
    }
    free(full_pathname);
  }

  fex_log("fstatat() returned %d\n", result);
  return result;
}