#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void test_file_ascii(const char *filename, int use_fex_library) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    printf("❌ Error: Cannot open file '%s'\n", filename);
    return;
  }

  int char_count = 0;
  int ascii_count = 0;
  int non_ascii_count = 0;
  int position = 0;

  unsigned char buffer[777];
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    for (size_t i = 0; i < bytes_read; i++) {
      unsigned char c = buffer[i];
      char_count++;
      position++;

      // Check if character is ASCII (0-127)
      int is_ascii = (c <= 127);

      if (is_ascii) {
        ascii_count++;
      } else {
        non_ascii_count++;

        // Display character info only on error
        char char_display[10];
        char description[50];

        if (isprint(c)) {
          snprintf(char_display, sizeof(char_display), "'%c'", c);
          snprintf(description, sizeof(description), "Printable");
        } else {
          snprintf(char_display, sizeof(char_display), "\\x%02X", c);
          switch (c) {
          case '\n':
            strcpy(description, "Newline");
            break;
          case '\r':
            strcpy(description, "Carriage Return");
            break;
          case '\t':
            strcpy(description, "Tab");
            break;
          case '\0':
            strcpy(description, "Null");
            break;
          case ' ':
            strcpy(description, "Space");
            break;
          default:
            if (c < 32) {
              snprintf(description, sizeof(description), "Control char (%d)",
                       c);
            } else {
              snprintf(description, sizeof(description), "Non-printable");
            }
            break;
          }
        }

        // Stop immediately and print error info
        printf("🚨 ERROR: Non-ASCII character detected in '%s'\n", filename);
        printf("📍 Position: %d\n", position);
        printf("🔍 Character: %s (0x%02X) - %s\n", char_display, c,
               description);
        if (use_fex_library) {
          printf("💡 Detected via FEX library interception\n");
        } else {
          printf("💡 Detected via standard file operations\n");
        }
        fclose(file);
        return;
      }
    }
  }

  fclose(file);

  // Only print summary if there were no errors (all ASCII)
  // Silent success - no output needed
}

void test_with_fex_library(const char *filename) {
  pid_t pid = fork();
  if (pid == 0) {
    // Reuse the inherited LD_PRELOAD value if the test runner already set it.
    const char *ld_preload = getenv("LD_PRELOAD");
    if (ld_preload && ld_preload[0] != '\0') {
      setenv("LD_PRELOAD", ld_preload, 1);
    } else {
      fprintf(stderr, "LD_PRELOAD is not set for the FEX child test\n");
      _exit(1);
    }
    setenv("FEX_DEBUG", "0", 1); // Disable debug to keep output clean

    // Execute our test using the current process directory.
    execl("./test_ascii_reader", "test_ascii_reader", filename, "fex", NULL);
    perror("exec failed");
    exit(1);
  } else if (pid > 0) {
    // Parent process: wait for child
    int status;
    waitpid(pid, &status, 0);
    if (WEXITSTATUS(status) != 0) {
      printf("❌ FEX library test failed with status %d\n",
             WEXITSTATUS(status));
    }
  } else {
    perror("fork failed");
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename> [fex]\n", argv[0]);
    printf("  filename: File to test for ASCII compliance\n");
    printf("  fex:      Optional flag to indicate FEX library is active\n");
    return 1;
  }

  const char *filename = argv[1];
  int fex_mode = (argc >= 3 && strcmp(argv[2], "fex") == 0);

  // Test the file
  test_file_ascii(filename, fex_mode);

  // If not in FEX mode and the file is a .fex file, also test with FEX library
  if (!fex_mode && strstr(filename, ".fex") != NULL) {
    // First test without FEX library (raw file)
    test_file_ascii(filename, 0);

    // Then test with FEX library
    test_with_fex_library(filename);
  }

  return 0;
}
