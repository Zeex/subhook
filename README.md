[SubHook][github] is a super-simple library for setting function hooks at run
time and it works on both Linux and Windows. Currently only x86 is supported
but the API is easily portable to other architectures.


Examples
--------

In the following examples `foo` is a hypothetical function or function pointer
that takes a single argument `int x` and uses the same calling convention as
`my_foo` (for instance, on x86 most compilers use the [`cdecl`][cdecl] calling
convention unless you specify otherwise).

### C

```C
#include <stdio.h>
#include <subhook.h>

struct subhook *foo_hook;

void my_foo(int x) {
  /* Sometimes you want to call the original function. */
  subhook_remove(foo_hook);

  printf("foo(%d) called\n", x);
  foo(x);

  /* Install the hook back again to intercept further calls. */
  subhook_install(foo_hook);
}

int main() {
  foo_hook = subhook_new();

  /* The "source" is the function that we want to hook. */
  subhook_set_src((void*)foo);

  /* The "destination" is the function that will get called instead of the
   * original function. */
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
#include <iostream>
#include <subhook.h>

SubHook foo_hook;

void my_foo(int x) {
  SubHook::ScopedRemove(&foo_hook);

  std::cout << "foo(" << x < ") called" << std::endl;
  foo(x);

  /* The hook wll be re-installed automatically upon return. */
}

int main() {
  hook.Install((void*)foo, (void*)my_foo);
}

```

[github]: https://github.com/Zeex/subhook
[cdecl]: http://en.wikipedia.org/wiki/X86_calling_conventions#cdecl
