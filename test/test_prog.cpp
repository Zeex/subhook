#include <subhook.h>

extern void foo();
extern void foo_hooked();

subhook::Hook foo_hook;

int main() {
  foo_hook.Install((void *)foo, (void *)foo_hooked);
  foo();
  foo_hook.Remove();
  foo();
  foo_hook.Install();
  foo();
  foo_hook.Remove();
  foo();
}
