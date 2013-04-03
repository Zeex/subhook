/* Copyright (c) 2012-2013 Zeex
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

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined _MSC_VER
	#if defined SUBHOOK_X86
		typedef __int32 intptr_t;
	#elif defined SUBHOOK_X86_64
		typedef __int64 intptr_t;
	#endif
#else
	#include <stdint.h>
#endif

#include "subhook.h"
#include "subhook_private.h"

#define SUBHOOK_JUMP_SIZE 5

struct subhook_x86 {
	unsigned char code[SUBHOOK_JUMP_SIZE];
};

int subhook_arch_new(struct subhook *hook) {
	if ((hook->arch = malloc(sizeof(struct subhook_x86))) == NULL)
		return -ENOMEM;

	return 0;
}

void subhook_arch_free(struct subhook *hook) {
	free(hook->arch);
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_install(struct subhook *hook) {
	static const unsigned char jmp = 0xE9;
	void *src, *dst;
	intptr_t offset;

	if (subhook_is_installed(hook))
		return -EINVAL;

	src = subhook_get_src(hook);
	dst = subhook_get_dst(hook);

	subhook_unprotect(src, SUBHOOK_JUMP_SIZE);
	memcpy(((struct subhook_x86 *)hook->arch)->code, src, SUBHOOK_JUMP_SIZE);

	/* E9 - jump near, relative */	
	memcpy(src, &jmp, sizeof(jmp));

	/* jump address is relative to next instruction */
	offset = (intptr_t)dst - ((intptr_t)src + SUBHOOK_JUMP_SIZE);
	memcpy((void*)((intptr_t)src + 1), &offset, SUBHOOK_JUMP_SIZE - sizeof(jmp));

	subhook_set_flags(hook, subhook_get_flags(hook) | SUBHOOK_FLAG_INSTALLED);

	return 0;
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_remove(struct subhook *hook) {
	if (!subhook_is_installed(hook))
		return -EINVAL;

	memcpy(subhook_get_src(hook), ((struct subhook_x86 *)hook->arch)->code, SUBHOOK_JUMP_SIZE);
	subhook_set_flags(hook, subhook_get_flags(hook) & ~(SUBHOOK_FLAG_INSTALLED));

	return 0;
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_read_dst(void *src) {
	if (*(unsigned char*)src == 0xE9)
		return (void *)(*(intptr_t *)((intptr_t)src + 1) + (intptr_t)src + SUBHOOK_JUMP_SIZE);

	return NULL;
}
