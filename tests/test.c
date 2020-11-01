#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <subhook.h>

typedef void (*foo_func_t)(void);

#ifdef SUBHOOK_X86
  #if defined SUBHOOK_WINDOWS
    #define FOO_CALL __cdecl
  #elif defined SUBHOOK_UNIX
    #define FOO_CALL __attribute__((cdecl))
  #endif
#endif
#ifndef FOO_CALL
  #define FOO_CALL
#endif

extern void FOO_CALL foo(void);
foo_func_t foo_tr = NULL;

void foo_hooked(void) {
  puts("foo_hooked() called");
}

void foo_hooked_tr(void) {
  puts("foo_hooked_tr() called");
  foo_tr();
}

int main() {
  puts("Testing initial install");

  subhook_t foo_hook = subhook_new((void *)foo,
                                   (void *)foo_hooked,
                                   SUBHOOK_64BIT_OFFSET);
  if (foo_hook == NULL || subhook_install(foo_hook) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }
  foo();
  if (subhook_remove(foo_hook) < 0) {
    puts("Remove failed");
    return EXIT_FAILURE;
  }
  foo();

  puts("Testing re-install");

  if (subhook_install(foo_hook) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }
  foo();
  if (subhook_remove(foo_hook) < 0) {
    puts("Remove failed");
    return EXIT_FAILURE;
  }
  foo();

  subhook_free(foo_hook);

  puts("Testing trampoline");

  subhook_t foo_hook_tr = subhook_new((void *)foo,
                                      (void *)foo_hooked_tr,
                                      SUBHOOK_64BIT_OFFSET);
  if (subhook_install(foo_hook_tr) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }
  foo_tr = (foo_func_t)subhook_get_trampoline(foo_hook_tr);
  if (foo_tr == NULL) {
    puts("Failed to build trampoline");
    return EXIT_FAILURE;
  }
  foo();

  subhook_free(foo_hook_tr);

  return EXIT_SUCCESS;
}
