#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Test framework for detecting memory corruption in FEX library */

/* Test result structure */
typedef struct {
  const char *test_name;
  int passed;
  char error_msg[512];
} test_result_t;

/* Global test results */
static test_result_t test_results[20];
static int test_count = 0;

/* Helper macros */
#define EXPECT_STR_EQ(expected, actual, msg)                                   \
  do {                                                                         \
    if (strcmp(expected, actual) != 0) {                                       \
      snprintf(test_results[test_count].error_msg,                             \
               sizeof(test_results[test_count].error_msg),                     \
               "%s: Expected '%s', got '%s'", msg, expected, actual);          \
      test_results[test_count].passed = 0;                                     \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define EXPECT_NO_CORRUPTION(buffer, size, msg)                                \
  do {                                                                         \
    if (detect_string_corruption(buffer, size)) {                              \
      snprintf(test_results[test_count].error_msg,                             \
               sizeof(test_results[test_count].error_msg),                     \
               "%s: String corruption detected", msg);                         \
      test_results[test_count].passed = 0;                                     \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define START_TEST(name)                                                       \
  do {                                                                         \
    test_results[test_count].test_name = name;                                 \
    test_results[test_count].passed = 1;                                       \
    test_results[test_count].error_msg[0] = '\0';                              \
  } while (0)

#define END_TEST()                                                             \
  do {                                                                         \
    test_count++;                                                              \
  } while (0)

/* Detect string corruption by looking for format specifiers and unexpected
 * patterns */
int detect_string_corruption(const char *buffer, size_t size) {
  /* Look for common corruption patterns */
  const char *corruption_patterns[] = {
      "%ld",   "%d",      "%s",    "%zu",     "%p",   "%c",   "%x",
      "%u",    "blocks=", "size=", "d_size=", "file", "load", "buffer",
      "track", "posit",   "simul", "EOF=",    NULL};

  for (int i = 0; corruption_patterns[i] != NULL; i++) {
    if (strstr(buffer, corruption_patterns[i]) != NULL) {
      printf("CORRUPTION: Found pattern '%s' in buffer\n",
             corruption_patterns[i]);
      return 1;
    }
  }

  /* Look for non-printable characters in hex data (except expected ones) */
  for (size_t i = 0; i < size; i++) {
    unsigned char c = buffer[i];
    /* Allow hex chars, comma, space, x, newline, and null terminator */
    if (c != 0 && c != '\n' && c != ' ' && c != ',' && c != 'x' &&
        !(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') &&
        !(c >= 'A' && c <= 'F') && c < 32) {
      printf("CORRUPTION: Found non-printable char 0x%02x at position %zu\n", c,
             i);
      return 1;
    }
  }

  return 0;
}

/* Test: Verify clean hex output from .fex file */
void test_clean_hex_output(void) {
  START_TEST("Clean hex output");

  /* Create a simple test .fex file */
  FILE *fp = fopen("/tmp/test_corruption.fex", "wb");
  if (!fp) {
    snprintf(test_results[test_count].error_msg,
             sizeof(test_results[test_count].error_msg),
             "Failed to create test file");
    test_results[test_count].passed = 0;
    END_TEST();
    return;
  }

  unsigned char test_data[] = {0x61, 0x00, 0x12, 0x5e, 0xf9, 0xff, 0x00, 0x01};
  fwrite(test_data, 1, sizeof(test_data), fp);
  fclose(fp);

  /* Use a child process to test with FEX library loaded */
  pid_t pid = fork();
  if (pid == 0) {
    /* Child process */
    setenv("LD_PRELOAD", "./build/src/libfex.so", 1);
    setenv("FEX_DEBUG", "0", 1);

    FILE *fex_fp = fopen("/tmp/test_corruption.fex", "r");
    if (!fex_fp) {
      exit(1);
    }

    char buffer[1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), fex_fp);
    fclose(fex_fp);

    /* Check for corruption */
    if (detect_string_corruption(buffer, bytes_read)) {
      exit(2); /* Corruption detected */
    }

    /* Verify expected hex patterns are present */
    if (!strstr(buffer, "0x61, ") || !strstr(buffer, "0xf9, ") ||
        !strstr(buffer, "0xff, ")) {
      exit(3); /* Expected patterns missing */
    }

    exit(0); /* Success */
  } else {
    /* Parent process */
    int status;
    waitpid(pid, &status, 0);

    if (WEXITSTATUS(status) == 1) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "Failed to open test file with FEX library");
      test_results[test_count].passed = 0;
    } else if (WEXITSTATUS(status) == 2) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "String corruption detected in output");
      test_results[test_count].passed = 0;
    } else if (WEXITSTATUS(status) == 3) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "Expected hex patterns missing from output");
      test_results[test_count].passed = 0;
    }
  }

  unlink("/tmp/test_corruption.fex");
  END_TEST();
}

/* Test: Verify hex table integrity */
void test_hex_table_integrity(void) {
  START_TEST("Hex table integrity");

  /* This test would need to access the internal hex table */
  /* For now, we test the output patterns */

  /* Create test file with known bytes */
  FILE *fp = fopen("/tmp/test_hextable.fex", "wb");
  if (!fp) {
    snprintf(test_results[test_count].error_msg,
             sizeof(test_results[test_count].error_msg),
             "Failed to create test file");
    test_results[test_count].passed = 0;
    END_TEST();
    return;
  }

  /* Test specific problematic bytes */
  unsigned char problematic_bytes[] = {0xf9, 0xfa, 0xfb, 0xfc,
                                       0xfd, 0xfe, 0xff};
  fwrite(problematic_bytes, 1, sizeof(problematic_bytes), fp);
  fclose(fp);

  pid_t pid = fork();
  if (pid == 0) {
    setenv("LD_PRELOAD", "./build/src/libfex.so", 1);
    setenv("FEX_DEBUG", "0", 1);

    FILE *fex_fp = fopen("/tmp/test_hextable.fex", "r");
    if (!fex_fp) {
      exit(1);
    }

    char buffer[1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), fex_fp);
    fclose(fex_fp);

    /* Check for specific hex values that were corrupted */
    char *expected_patterns[] = {"0xf9, ", "0xfa, ", "0xfb, ", "0xfc, ",
                                 "0xfd, ", "0xfe, ", "0xff, ", NULL};

    for (int i = 0; expected_patterns[i]; i++) {
      if (!strstr(buffer, expected_patterns[i])) {
        printf("MISSING: Expected pattern '%s' not found\n",
               expected_patterns[i]);
        exit(3);
      }
    }

    /* Check for corruption */
    if (detect_string_corruption(buffer, bytes_read)) {
      exit(2);
    }

    exit(0);
  } else {
    int status;
    waitpid(pid, &status, 0);

    if (WEXITSTATUS(status) != 0) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "Hex table integrity test failed with code %d",
               WEXITSTATUS(status));
      test_results[test_count].passed = 0;
    }
  }

  unlink("/tmp/test_hextable.fex");
  END_TEST();
}

/* Test: Memory boundary checking */
void test_memory_boundaries(void) {
  START_TEST("Memory boundaries");

  /* Create a file that will test boundary conditions */
  FILE *fp = fopen("/tmp/test_boundaries.fex", "wb");
  if (!fp) {
    snprintf(test_results[test_count].error_msg,
             sizeof(test_results[test_count].error_msg),
             "Failed to create test file");
    test_results[test_count].passed = 0;
    END_TEST();
    return;
  }

  /* Write data that straddles block boundaries */
  unsigned char boundary_data[8192]; /* Larger than default block size */
  for (int i = 0; i < sizeof(boundary_data); i++) {
    boundary_data[i] = (i % 256); /* Include all possible byte values */
  }
  fwrite(boundary_data, 1, sizeof(boundary_data), fp);
  fclose(fp);

  pid_t pid = fork();
  if (pid == 0) {
    setenv("LD_PRELOAD", "./build/src/libfex.so", 1);
    setenv("FEX_DEBUG", "0", 1);
    setenv("FEX_BLOCK_SIZE", "1024",
           1); /* Use smaller block size to test boundaries */

    FILE *fex_fp = fopen("/tmp/test_boundaries.fex", "r");
    if (!fex_fp) {
      exit(1);
    }

    /* Read in chunks to test boundary conditions */
    char buffer[16384];
    size_t total_read = 0;
    size_t chunk_size = 100;

    while (total_read < sizeof(buffer) - chunk_size) {
      size_t bytes_read = fread(buffer + total_read, 1, chunk_size, fex_fp);
      if (bytes_read == 0)
        break;
      total_read += bytes_read;
    }

    fclose(fex_fp);

    /* Check for corruption in the entire buffer */
    if (detect_string_corruption(buffer, total_read)) {
      exit(2);
    }

    exit(0);
  } else {
    int status;
    waitpid(pid, &status, 0);

    if (WEXITSTATUS(status) != 0) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "Memory boundary test failed with code %d", WEXITSTATUS(status));
      test_results[test_count].passed = 0;
    }
  }

  unlink("/tmp/test_boundaries.fex");
  END_TEST();
}

/* Test: Stress test with random data */
void test_random_data_corruption(void) {
  START_TEST("Random data corruption");

  FILE *fp = fopen("/tmp/test_random.fex", "wb");
  if (!fp) {
    snprintf(test_results[test_count].error_msg,
             sizeof(test_results[test_count].error_msg),
             "Failed to create test file");
    test_results[test_count].passed = 0;
    END_TEST();
    return;
  }

  /* Generate random data */
  srand(12345); /* Fixed seed for reproducible tests */
  unsigned char random_data[4096];
  for (int i = 0; i < sizeof(random_data); i++) {
    random_data[i] = rand() % 256;
  }
  fwrite(random_data, 1, sizeof(random_data), fp);
  fclose(fp);

  pid_t pid = fork();
  if (pid == 0) {
    setenv("LD_PRELOAD", "./build/src/libfex.so", 1);
    setenv("FEX_DEBUG", "0", 1);

    FILE *fex_fp = fopen("/tmp/test_random.fex", "r");
    if (!fex_fp) {
      exit(1);
    }

    char buffer[32768];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), fex_fp);
    fclose(fex_fp);

    /* Check for corruption */
    if (detect_string_corruption(buffer, bytes_read)) {
      exit(2);
    }

    exit(0);
  } else {
    int status;
    waitpid(pid, &status, 0);

    if (WEXITSTATUS(status) != 0) {
      snprintf(test_results[test_count].error_msg,
               sizeof(test_results[test_count].error_msg),
               "Random data corruption test failed with code %d",
               WEXITSTATUS(status));
      test_results[test_count].passed = 0;
    }
  }

  unlink("/tmp/test_random.fex");
  END_TEST();
}

/* Print test results */
void print_test_results(void) {
  int passed = 0;
  int failed = 0;

  printf("\n=== FEX Memory Corruption Test Results ===\n");

  for (int i = 0; i < test_count; i++) {
    if (test_results[i].passed) {
      printf("✅ PASS: %s\n", test_results[i].test_name);
      passed++;
    } else {
      printf("❌ FAIL: %s\n", test_results[i].test_name);
      printf("   Error: %s\n", test_results[i].error_msg);
      failed++;
    }
  }

  printf("\n=== Summary ===\n");
  printf("Total tests: %d\n", test_count);
  printf("Passed: %d\n", passed);
  printf("Failed: %d\n", failed);

  if (failed > 0) {
    printf("\n🚨 MEMORY CORRUPTION DETECTED! 🚨\n");
    printf("The FEX library has memory corruption issues that need to be "
           "fixed.\n");
  } else {
    printf("\n✅ All memory corruption tests passed!\n");
  }
}

int main(int argc, char *argv[]) {
  printf("FEX Memory Corruption Test Framework\n");
  printf("===================================\n\n");

  /* Check if library exists */
  if (access("./build/src/libfex.so", F_OK) != 0) {
    printf("ERROR: FEX library not found. Please build the project first.\n");
    printf("Run: make\n");
    return 1;
  }

  /* Run all tests */
  test_clean_hex_output();
  test_hex_table_integrity();
  test_memory_boundaries();
  test_random_data_corruption();

  /* Print results */
  print_test_results();

  /* Return appropriate exit code */
  int failed_tests = 0;
  for (int i = 0; i < test_count; i++) {
    if (!test_results[i].passed) {
      failed_tests++;
    }
  }

  return failed_tests > 0 ? 1 : 0;
}