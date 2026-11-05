#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  printf("Testing openat call...\n");

  int fd = openat(AT_FDCWD, "tests/fixtures/simple_test.fex", O_RDONLY);
  if (fd == -1) {
    printf("openat failed: %s\n", strerror(errno));
    return 1;
  }

  printf("openat returned fd: %d\n", fd);

  char buffer[100];
  ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
  if (bytes_read > 0) {
    buffer[bytes_read] = '\0';
    printf("First %zd bytes: %s\n", bytes_read, buffer);
  }

  close(fd);
  printf("Done\n");
  return 0;
}
