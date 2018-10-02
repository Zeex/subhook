#include <cstdlib>
#include <iostream>
#include <subhook.h>

typedef void (*foo_func_t)();

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

extern "C" void FOO_CALL foo();
foo_func_t foo_tr = 0;

void foo_hooked() {
  std::cout << "foo_hooked() called" << std::endl;;
}

void foo_hooked_tr() {
  std::cout << "foo_hooked_tr() called" << std::endl;
  foo_tr();
}

int main() {
  std::cout << "Testing initial install" << std::endl;

  subhook::Hook foo_hook((void *)foo,
                         (void *)foo_hooked,
                         subhook::HookFlag64BitOffset);
  if (!foo_hook.Install()) {
    std::cout << "Install failed" << std::endl;
    return EXIT_FAILURE;
  }
  foo();
  if (!foo_hook.Remove()) {
    std::cout << "Remove failed" << std::endl;
    return EXIT_FAILURE;
  }
  foo();

  std::cout << "Testing re-install" << std::endl;

  if (!foo_hook.Install()) {
    std::cout << "Install failed" << std::endl;
    return EXIT_FAILURE;
  }
  foo();
  if (!foo_hook.Remove()) {
    std::cout << "Remove failed" << std::endl;
    return EXIT_FAILURE;
  }
  foo();

  std::cout << "Testing trampoline" << std::endl;

  subhook::Hook foo_hook_tr((void *)foo,
                            (void *)foo_hooked_tr,
                            subhook::HookFlag64BitOffset);
  if (!foo_hook_tr.Install()) {
    std::cout << "Install failed" << std::endl;
    return EXIT_FAILURE;
  }
  foo_tr = (foo_func_t)foo_hook_tr.GetTrampoline();
  if (foo_tr == 0) {
    std::cout << "Failed to build trampoline" << std::endl;
    return EXIT_FAILURE;
  }
  foo();

  return EXIT_SUCCESS;
}
