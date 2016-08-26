#include <cstdio>
#include <subhook.h>

extern void foo();
extern void foo_hooked();

typedef void (*foo_func_t)();

subhook::Hook foo_hook;

int main() {
  std::printf("Testing initial install\n");

  foo_hook.Install((void *)foo,
                   (void *)foo_hooked,
                   subhook::HookOption64BitOffset);
  foo();
  foo_hook.Remove();
  foo();

  std::printf("Testing re-install\n");
  foo_hook.Install();
  foo();
  foo_hook.Remove();
  foo();

  std::printf("Testing trampoline\n");
  foo_func_t foo_ptr = (foo_func_t)foo_hook.GetTrampoline();
  foo_ptr();
}
