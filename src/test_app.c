#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  printf("Test application starting...\n");
  printf("This application will perform various file I/O operations\n");
  printf("that should be intercepted by the FEX LD_PRELOAD library.\n\n");

  /* Test file descriptor operations */
  printf("=== Testing file descriptor operations ===\n");
  int fd = open("/etc/passwd", O_RDONLY);
  if (fd >= 0) {
    char buffer[256];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
      buffer[bytes] = '\0';
      printf("Read %zd bytes from /etc/passwd\n", bytes);
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == 0) {
      printf("File size: %ld bytes\n", statbuf.st_size);
    }

    close(fd);
  }

  /* Test FILE* operations */
  printf("\n=== Testing FILE* operations ===\n");
  FILE *fp = fopen("/etc/hostname", "r");
  if (fp) {
    char hostname[256];
    if (fgets(hostname, sizeof(hostname), fp)) {
      printf("Hostname: %s", hostname);
    }

    rewind(fp);
    int c = fgetc(fp);
    printf("First character: %c\n", c);

    if (!feof(fp)) {
      printf("Not at end of file\n");
    }

    printf("File descriptor: %d\n", fileno(fp));
    fclose(fp);
  }

  /* Test stat operations */
  printf("\n=== Testing stat operations ===\n");
  struct stat statbuf;
  if (stat("/tmp", &statbuf) == 0) {
    printf("/tmp is a %s\n", S_ISDIR(statbuf.st_mode) ? "directory" : "file");
  }

  /* Test .fex file tracking */
  printf("\n=== Testing .fex file tracking ===\n");

  /* Create test .fex files */
  const char *fex_file1 = "/tmp/test1.fex";
  const char *fex_file2 = "/tmp/test2.fex";
  const char *regular_file = "/tmp/test.txt";

  /* Test with file descriptors */
  printf("Creating .fex files with file descriptors...\n");
  int fd1 = open(fex_file1, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd1 >= 0) {
    write(fd1, "Hello from test1.fex\n", 21);
    close(fd1);
  }

  int fd2 = open(fex_file2, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd2 >= 0) {
    write(fd2, "Hello from test2.fex - this is a longer file\n", 45);
    close(fd2);
  }

  /* Test with FILE pointers */
  printf("Creating files with FILE pointers...\n");
  FILE *fp1 = fopen(fex_file1, "a");
  if (fp1) {
    fprintf(fp1, "Appended text\n");
    fclose(fp1);
  }

  /* Test regular file (should not be tracked) */
  FILE *fp_regular = fopen(regular_file, "w");
  if (fp_regular) {
    fprintf(fp_regular, "This is a regular file, not .fex\n");
    fclose(fp_regular);
  }

  /* Test reading .fex files */
  printf("Reading .fex files...\n");
  FILE *fp2 = fopen(fex_file2, "r");
  if (fp2) {
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), fp2)) {
      printf("Content from %s: %s", fex_file2, buffer);
    }

    printf("(Check debug output for .fex file tracking status)\n");
    fclose(fp2);
  }

  /* Cleanup */
  unlink(fex_file1);
  unlink(fex_file2);
  unlink(regular_file);

  printf("\nTest application completed.\n");
  return 0;
}