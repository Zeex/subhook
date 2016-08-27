#include <cstdio>
#include <subhook.h>

typedef void (*foo_func_t)();

foo_func_t foo_tr = 0;

void foo() {
  std::printf("foo() called\n");
}

void foo_hooked() {
  std::printf("foo_hooked() called\n");
}

void foo_hooked_tr() {
  std::printf("foo_hooked_tr() called\n");
  foo_tr();
}

int main() {
  std::printf("Testing initial install\n");

  subhook::Hook foo_hook;
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

  subhook::Hook foo_hook_tr((void *)foo,
                            (void *)foo_hooked_tr,
                            subhook::HookOption64BitOffset);
  foo_hook_tr.Install();
  foo_tr = (foo_func_t)foo_hook_tr.GetTrampoline();
  foo();
}
