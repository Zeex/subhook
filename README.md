[subhook][link] is a super-simple mini-library for setting function hooks at
runtime on x86 CPUs and works on both Linux and Windows.

[link]: https://github.com/Zeex/subhook

Examples
--------

### C

```C
#include <subhook.h>

struct subhook *foo_hook;

void my_foo() {
  /* Sometimes you want to call the original function. */
  subhook_remove(foo_hook);

  printf("foo() called\n");
  foo();

  /* Install the hook back again to intercept further calls. */
  subhook_install(foo_hook);
}

int main() {
  foo_hook = subhook_new();

  /* 'source' is the function that we want to hook. */
  subhook_set_src((void*)foo);

  /* 'destination' is the function that will be called in place
   * of the original function */
  subhook_set_dst((void*)my_foo);

  /* Install our newly created hook so from now on any call to foo()
   * will be redirected to my_foo(). */ 
  subhook_install(foo_hook);
  
  /* Free the memory when you're done. */
  subhook_free(foo_hook);
}
```

### C++

```C++
#include <subhook.h>

SubHook foo_hook;

void my_foo() {
  SubHook::ScopedRemove(&foo_hook);

  printf("foo() called\n");
  foo();

  /* The hook wll be re-installed automatically upon return. */
}

int main() {
  hook.Install((void*)foo, (void*)my_foo);
}

```
