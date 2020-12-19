[![Build Status][build_status]][build]
[![Build Status - Windows][build_status_win]][build_win]

SubHook is a super-simple hooking library for C and C++ that works on Windows,
Linux and macOS. It supports x86 only (32-bit and 64-bit).

Installation
------------

Easy method:

1. Copy the source and header files to your project and include
   [`subhook.c`](subhook.c) in your build.
2. On Windows only: Define `SUBHOOK_STATIC` before including `subhook.h`.

With CMake:

1. Copy the subhook repo to your project tree.
2. Call `add_subdirectory(path/to/subhook)` in your CMakeLists.txt.
3. Optional: configure how the library is built by setting these varaible prior
   to `add_subdirectory(...)`:

   * `SUBHOOK_STATIC` - Build as static library (`OFF` by default)
   * `SUBHOOK_INSTALL` - Enable installation and packaging of targets/files
     with CPack (`OFF` by default)
   * `SUBHOOK_TESTS` - Enable tests (`ON` by default)
   * `SUBHOOK_FORCE_32BIT` - Configure for compiling 32-bit binaries on 64-bit
     systems (default is `OFF`)

Use of CMake is not mandatory, the library can be built without it (no extra
build configuration is required).

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
  foo_hook = subhook_new((void *)foo, (void *)my_foo, 0);

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
   /* Same code as in the previous example. */
}
```

Please note that subhook has a very simple length disassmebler engine (LDE)
that works only with most common prologue instructions like push, mov, call,
etc. When it encounters an unknown instruction subhook_get_trampoline() will
return NULL. You can delegate instruction decoding to a custom disassembler
of your choice via `subhook_set_disasm_handler()`.

### C++

```c++
#include <iostream>
#include <subhook.h>

subhook::Hook foo_hook;
subhook::Hook foo_hook_tr;

typedef void (*foo_func)(int x);

void my_foo(int x) {
  // ScopedHookRemove removes the specified hook and automatically re-installs
  // it when the object goes out of scope (thanks to C++ destructors).
  subhook::ScopedHookRemove remove(&foo_hook);

  std::cout << "foo(" << x << ") called" << std::endl;
  foo(x + 1);
}

void my_foo_tr(int x) {
  std::cout << "foo(" << x << ") called" << std::endl;

  // Call the original function via trampoline.
  ((foo_func)foo_hook_tr.GetTrampoline())(x + 1);
}

int main() {
  foo_hook.Install((void *)foo, (void *)my_foo);
  foo_hook_tr.Install((void *)foo, (void *)my_foo_tr);
}
```

Known issues
------------

* `subhook_get_trampoline()` may return NULL because only a small subset of
  x86 instructions is supported by the disassembler in this library (just
  common prologue instructions). As a workaround you can plug in a more
  advanced instruction length decoder using `subhook_set_disasm_handler()`.

* If a target function (the function you are hooking) is less than N bytes
  in length, for example if it's a short 2-byte jump to a nearby location
  (sometimes compilers generate code like this), then you will not be able
  to hook it.

  N is 5 by default: 1 byte for jmp opcode + 4 bytes for offset. But if you
  enable the use of 64-bit offsets in 64-bit mode N becomes 14 (see the
  definition of `subhook_jmp64`).

* Some systems protect executable code form being modified at runtime, which
  will not allow you to install hooks, or don't allow to mark heap-allocated
  memory as executable, which prevents the use of trampolines.

  For example, on Fedora you can have such problems because of SELinux (though
  you can disable it or exclude your files). On macOS Catalina the
  `mprotect()` call inside `subhook_new` will fail with "Permission denied"
  (see https://github.com/Zeex/subhook/issues/45).

License
-------

Licensed under the 2-clause BSD license.

[build]: https://travis-ci.org/Zeex/subhook
[build_status]: https://travis-ci.org/Zeex/subhook.svg?branch=master
[build_win]: https://ci.appveyor.com/project/Zeex/subhook/branch/master
[build_status_win]: https://ci.appveyor.com/api/projects/status/q5sp0p8ahuqfh8e4/branch/master?svg=true
