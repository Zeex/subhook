[![Version][version_badge]][version]
[![Build Status][build_status]][build]
[![Build Status - Windows][build_status_win]][build_win]

SubHook is a super-simple hooking library for C/C++ that works on Linux and
Windows. It currently supports x86 and x86-64.

Examples
--------

In the following examples `foo` is some function or a function pointer that
takes a single argument of type `int` and uses the same calling convention
as `my_foo` (depends on compiler).

### Basic usage

```c
#include <stdio.h>
#include <subhook.h>

subhook_t foo_hook;

void my_foo(int x) {
  /* Remove the hook so that you can call the original function. */
  subhook_remove(foo_hook);

  printf("foo(%d) called\n", x);
  foo(x);

  /* Install the hook back to intercept further calls. */
  subhook_install(foo_hook);
}

int main() {
  /* Create a hook that will redirect all foo() calls to to my_foo(). */
  foo_hook = subhook_new((void *)foo, (void *)my_foo);

  /* Install it. */
  subhook_install(foo_hook);

  foo(123);

  /* Remove the hook and free memory when you're done. */
  subhook_remove(foo_hook);
  subhook_free(foo_hook);
}
```

### Trampolines

Using trampolines allows you to jump to the original code without removing
and re-installing hooks every time your function gets called.

```c
typedef void (*foo_func)(int x);

void my_foo(int x) {
  printf("foo(%d) called\n", x);

  /* Call foo() via trampoline. */
  ((foo_func)subhook_get_trampoline(foo_hook))(x);
}

int main() {
   /* Same code as in previous example. */
}
```

Please note that subhook has a very simple length disassmebler engine (LDE) that works only with most common prologue instructions like push, mov, call, etc. When it encounters an unknown instruction subhook_get_trampoline() will return NULL.

### C++

```c++
#include <iostream>
#include <subhook.h>

SubHook foo_hook;
SubHook foo_hook_tr;

typedef void (*foo_func)(int x);

void my_foo(int x) {
  // ScopedRemove removes the specified hook and automatically re-installs it
  // when the objectt goes out of scope (thanks to C++ destructors).
  SubHook::ScopedRemove remove(&foo_hook);

  std::cout << "foo(" << x < ") called" << std::endl;
  foo(x + 1);
}

void my_foo_tr(int x) {
  std::cout << "foo(" << x < ") called" << std::endl;

  // Call the original function via trampoline.
  ((foo_func)foo_hook_tr.GetTrampoline())(x + 1);
}

int main() {
  foo_hook.Install((void *)foo, (void *)my_foo);
  foo_hook_tr.Install((void *)foo, (void *)my_foo_tr);
}
```

License
-------

Licensed under the 2-clause BSD license.

[version]: http://badge.fury.io/gh/zeex%2Fsubhook
[version_badge]: https://badge.fury.io/gh/zeex%2Fsubhook.svg
[build]: https://travis-ci.org/Zeex/subhook
[build_status]: https://travis-ci.org/Zeex/subhook.svg?branch=master
[build_win]: https://ci.appveyor.com/project/Zeex/subhook/branch/master
[build_status_win]: https://ci.appveyor.com/api/projects/status/q5sp0p8ahuqfh8e4/branch/master?svg=true
