#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

int main() {
  printf("Testing FEX LD_PRELOAD library loading...\n");

  /* Try to load the library */
  void *handle = dlopen("../src/libfex.so", RTLD_LAZY);
  if (!handle) {
    printf("Cannot load library: %s\n", dlerror());
    return 1;
  }

  printf("Library loaded successfully!\n");

  /* Try to find the fex_init symbol */
  void (*fex_init)() = dlsym(handle, "fex_init");
  if (fex_init) {
    printf("Found fex_init symbol\n");
    fex_init();
    printf("fex_init() called successfully\n");
  } else {
    printf("Warning: fex_init symbol not found\n");
  }

  dlclose(handle);
  printf("All tests passed!\n");
  return 0;
}