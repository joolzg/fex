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

  printf("\nTest application completed.\n");
  return 0;
}