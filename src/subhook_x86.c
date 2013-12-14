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
	typedef __int32 int32_t;
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

#define JMP_OPCODE 0xE9

static const unsigned char jmp_opcode = JMP_OPCODE;
static const unsigned char jmp_instr[] = { JMP_OPCODE, 0x0, 0x0, 0x0, 0x0 };

struct subhook_x86 {
	struct subhook _;
	unsigned char code[sizeof(jmp_instr)];
};

SUBHOOK_EXPORT subhook_t SUBHOOK_API subhook_new() {
	struct subhook_x86 *hook;

	if ((hook = calloc(1, sizeof(*hook))) == NULL)
		return NULL;

	return (subhook_t)hook;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_free(subhook_t hook) {
	free(hook);
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_install(subhook_t hook) {
	void *src;
	void *dst;
	intptr_t offset;

	if (hook->installed)
		return -EINVAL;

	src = hook->src;
	dst = hook->dst;

	subhook_unprotect(src, sizeof(jmp_instr));
	memcpy(((struct subhook_x86 *)hook)->code, src, sizeof(jmp_instr));
	memcpy(src, &jmp_instr, sizeof(jmp_instr));

	offset = (intptr_t)dst - ((intptr_t)src + sizeof(jmp_instr));
	memcpy((void *)((intptr_t)src + sizeof(jmp_opcode)), &offset,
	       sizeof(jmp_instr) - sizeof(jmp_opcode));

	hook->installed = 1;
	return 0;
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_remove(subhook_t hook) {
	if (!hook->installed)
		return -EINVAL;

	memcpy(hook->src, ((struct subhook_x86 *)hook)->code,
	       sizeof(jmp_instr));

	hook->installed = 0;
	return 0;
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_read_dst(void *src) {
	unsigned char opcode;
	int32_t offset;

	memcpy(&opcode, src, sizeof(opcode));
	if (opcode != jmp_opcode)
		return NULL;

	memcpy(&offset, (void *)((intptr_t)src + sizeof(jmp_opcode)),
	       sizeof(offset));
	return (void *)(offset + (intptr_t)src + sizeof(jmp_instr));
}
