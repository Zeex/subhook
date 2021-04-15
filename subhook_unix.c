/*
 * Copyright (c) 2012-2018 Zeex
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "subhook_private.h"

#define SUBHOOK_CODE_PROTECT_FLAGS (PROT_READ | PROT_WRITE | PROT_EXEC)

int subhook_unprotect(void *address, size_t size) {
  long pagesize;

  pagesize = sysconf(_SC_PAGESIZE);
  address = (void *)((long)address & ~(pagesize - 1));

  return mprotect(address, size, SUBHOOK_CODE_PROTECT_FLAGS);
}


void *subhook_alloc_code(void* src_addr, size_t size, subhook_flags_t flags) {
  void *address;
#if defined __linux__ && defined MAP_FIXED_NOREPLACE && defined SUBHOOK_X86_64
  if ((flags & SUBHOOK_TRY_ALLOCATE_TRAMPOLINE_NEAR_SOURCE) != 0) {
    // Go over /proc/<pid>/maps to find closeby unmapped pages.
    void* preferred_addr = NULL;
    char maps_fpath[40];
    sprintf(maps_fpath, "/proc/%ld/maps", (int64_t)getpid());
    FILE* fp = fopen(maps_fpath, "r");
    if (fp != NULL) {
      int64_t mapped_begin = 0, mapped_end = 0;
      while (!feof(fp)) {
        int64_t prev_mapped_end = mapped_end;
        fscanf(fp, "%lx-%lx", &mapped_begin, &mapped_end);

        // Assume the /proc/<pid>/maps file is sorted by mem addr.
        // Find first unmapped mem range above src_addr.
        if ((int64_t)src_addr <= prev_mapped_end && prev_mapped_end < mapped_begin) {
          preferred_addr = (void*)prev_mapped_end;
          break;
        }

        // We only need begin/end of mem range, skip other info in the line.
        while (!feof(fp) && fgetc(fp) != '\n') {
          ;
        }
      }
      fclose(fp);
    }

    if (preferred_addr != NULL) {
      address = mmap(preferred_addr,
                     size,
                     SUBHOOK_CODE_PROTECT_FLAGS,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
                     -1,
                     0);
      if (address != MAP_FAILED) {
        return address;
      }
    }
  }
#endif // __linux__ && MAP_FIXED_NOREPLACE && SUBHOOK_X86_64

  address = mmap(NULL,
                 size,
                 SUBHOOK_CODE_PROTECT_FLAGS,
                 #if defined MAP_32BIT && !defined __APPLE__
                   MAP_32BIT |
                 #endif
                 MAP_PRIVATE | MAP_ANONYMOUS,
                 -1,
                 0);
  return address == MAP_FAILED ? NULL : address;
}

int subhook_free_code(void *address, size_t size) {
  if (address == NULL) {
    return 0;
  }
  return munmap(address, size);
}
