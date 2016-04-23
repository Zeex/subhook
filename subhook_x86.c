/* Copyright (c) 2012-2015 Zeex
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

#include "subhook.h"
#include "subhook_private.h"

#ifdef SUBHOOK_WINDOWS
	typedef unsigned __int8 uint8_t;
	typedef __int32 int32_t;
	typedef unsigned __int32 uint32_t;
	#if SUBHOOK_BITS == 32
		typedef __int32 intptr_t;
		typedef unsigned __int32 uintptr_t;
	#else
		typedef __int64 intptr_t;
		typedef unsigned __int64 uintptr_t;
	#endif
#else
	#include <stdint.h>
#endif

#define JMP_OPCODE  0xE9
#define PUSH_OPCODE 0x68
#define MOV_OPCODE  0xC7
#define RET_OPCODE  0xC3

#define MOV_MODRM_BYTE 0x44 /* write to address + 1 byte displacement */
#define MOV_SIB_BYTE   0x24 /* write to [rsp] */
#define MOV_OFFSET     0x04

#pragma pack(push, 1)

struct subhook_jmp32 {
	uint8_t opcode;
	int32_t offset;
};

/* Since AMD64 doesn't support 64-bit direct jumps, we'll push the address
 * onto the stack, then call RET.
 */
struct subhook_jmp64 {
	uint8_t  push_opcode;
	uint32_t push_addr; /* lower 32-bits of the address to jump to */
	uint8_t  mov_opcode;
	uint8_t  mov_modrm;
	uint8_t  mov_sib;
	uint8_t  mov_offset;
	uint32_t mov_addr;  /* upper 32-bits of the address to jump to */
	uint8_t  ret_opcode;
};

#pragma pack(pop)

#if SUBHOOK_BITS == 32
	#define JMP_SIZE sizeof(struct subhook_jmp32)
#else
	#define JMP_SIZE sizeof(struct subhook_jmp64)
#endif

#define MAX_INSN_LEN 15
#define MAX_TRAMPOLINE_LEN (JMP_SIZE + MAX_INSN_LEN - 1)

static int subhook_disasm(void *src, int32_t *reloc_op_offset) {
	enum flags {
		MODRM      = 1,
		PLUS_R     = 1 << 1,
		REG_OPCODE = 1 << 2,
		IMM8       = 1 << 3,
		IMM16      = 1 << 4,
		IMM32      = 1 << 5,
		RELOC      = 1 << 6
	};

	static int prefixes[] = {
		0xF0, 0xF2, 0xF3,
		0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65,
		0x66, /* operand size override */
		0x67  /* address size override */
	};

	struct opcode_info {
		int opcode;
		int reg_opcode;
		int flags;
	};

	/*
	 * Refer to the Intel Developer Manual volumes 2a and 2b for more information
	 * about instruction formats and encoding:
	 *
	 * https://www-ssl.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
	 */
	static struct opcode_info opcodes[] = {
		/* CALL rel32        */ {0xE8, 0, IMM32 | RELOC},
		/* CALL r/m32        */ {0xFF, 2, MODRM | REG_OPCODE},
		/* JMP rel32         */ {0xE9, 0, IMM32 | RELOC},
		/* JMP r/m32         */ {0xFF, 4, MODRM | REG_OPCODE},
		/* LEA r16,m         */ {0x8D, 0, MODRM},
		/* MOV r/m8,r8       */ {0x88, 0, MODRM},
		/* MOV r/m32,r32     */ {0x89, 0, MODRM},
		/* MOV r8,r/m8       */ {0x8A, 0, MODRM},
		/* MOV r32,r/m32     */ {0x8B, 0, MODRM},
		/* MOV r/m16,Sreg    */ {0x8C, 0, MODRM},
		/* MOV Sreg,r/m16    */ {0x8E, 0, MODRM},
		/* MOV AL,moffs8     */ {0xA0, 0, IMM8},
		/* MOV EAX,moffs32   */ {0xA1, 0, IMM32},
		/* MOV moffs8,AL     */ {0xA2, 0, IMM8},
		/* MOV moffs32,EAX   */ {0xA3, 0, IMM32},
		/* MOV r8, imm8      */ {0xB0, 0, PLUS_R | IMM8},
		/* MOV r32, imm32    */ {0xB8, 0, PLUS_R | IMM32},
		/* MOV r/m8, imm8    */ {0xC6, 0, MODRM | REG_OPCODE | IMM8},
		/* MOV r/m32, imm32  */ {0xC7, 0, MODRM | REG_OPCODE | IMM32},
		/* POP r/m32         */ {0x8F, 0, MODRM | REG_OPCODE},
		/* POP r32           */ {0x58, 0, PLUS_R},
		/* PUSH r/m32        */ {0xFF, 6, MODRM | REG_OPCODE},
		/* PUSH r32          */ {0x50, 0, PLUS_R},
		/* PUSH imm8         */ {0x6A, 0, IMM8},
		/* PUSH imm32        */ {0x68, 0, IMM32},
		/* RET               */ {0xC3, 0, 0},
		/* RET imm16         */ {0xC2, 0, IMM16},
		/* SUB AL, imm8      */ {0x2C, 0, IMM8},
		/* SUB EAX, imm32    */ {0x2D, 0, IMM32},
		/* SUB r/m8, imm8    */ {0x80, 5, MODRM | REG_OPCODE | IMM8},
		/* SUB r/m32, imm32  */ {0x81, 5, MODRM | REG_OPCODE | IMM8},
		/* SUB r/m32, imm8   */ {0x83, 5, MODRM | REG_OPCODE | IMM8},
		/* SUB r/m32, r32    */ {0x29, 0, MODRM},
		/* SUB r32, r/m32    */ {0x2B, 0, MODRM},
		/* TEST AL, imm8     */ {0xA8, 0, IMM8},
		/* TEST EAX, imm32   */ {0xA9, 0, IMM32},
		/* TEST r/m8, imm8   */ {0xF6, 0, MODRM | REG_OPCODE | IMM8},
		/* TEST r/m32, imm32 */ {0xF7, 0, MODRM | REG_OPCODE | IMM32},
		/* TEST r/m8, r8     */ {0x84, 0, MODRM},
		/* TEST r/m32, r32   */ {0x85, 0, MODRM}
	};

	uint8_t *code = src;
	int i;
	int len = 0;
	int operand_size = 4;
	int address_size = 4;
	int opcode = 0;

	for (i = 0; i < sizeof(prefixes) / sizeof(*prefixes); i++) {
		if (code[len] == prefixes[i]) {
			len++;
			if (prefixes[i] == 0x66)
				operand_size = 2;
			if (prefixes[i] == 0x67)
				address_size = SUBHOOK_BITS / 8 / 2;
		}
	}

	for (i = 0; i < sizeof(opcodes) / sizeof(*opcodes); i++) {
		int found = 0;

		if (code[len] == opcodes[i].opcode)
			found = !(opcodes[i].flags & REG_OPCODE)
				|| ((code[len + 1] >> 3) & 7) == opcodes[i].reg_opcode;

		if ((opcodes[i].flags & PLUS_R)
			&& (code[len] & 0xF8) == opcodes[i].opcode)
			found = 1;

		if (found) {
			opcode = code[len++];
			break;
		}
	}

	if (opcode == 0)
		return 0;

	if (reloc_op_offset != NULL && opcodes[i].flags & RELOC)
		*reloc_op_offset = len; /* relative call or jump */

	if (opcodes[i].flags & MODRM) {
		int modrm = code[len++];
		int mod = modrm >> 6;
		int rm = modrm & 7;

		if (mod != 3 && rm == 4)
			len++; /* for SIB */

#if SUBHOOK_BITS == 64
		if (reloc_op_offset != NULL && rm == 5)
			*reloc_op_offset = len; /* RIP-relative addressing */
#endif

		if (mod == 1)
			len += 1; /* for disp8 */
		if (mod == 2 || (mod == 0 && rm == 5))
			len += 4; /* for disp32 */
	}

	if (opcodes[i].flags & IMM8)
		len += 1;
	if (opcodes[i].flags & IMM16)
		len += 2;
	if (opcodes[i].flags & IMM32)
		len += operand_size;

	return len;
}

#if SUBHOOK_BITS == 32

static void subhook_make_jmp(void *src, void *dst) {
	struct subhook_jmp32 *jmp = (struct subhook_jmp32 *)src;

	jmp->opcode = JMP_OPCODE;
	jmp->offset = (int32_t)((intptr_t)dst - ((intptr_t)src + JMP_SIZE));
}

#else

static void subhook_make_jmp(void *src, void *dst) {
	struct subhook_jmp64 *jmp = (struct subhook_jmp64 *)src;

	jmp->push_opcode = PUSH_OPCODE;
	jmp->push_addr = (uint32_t)dst; /* truncate */
	jmp->mov_opcode = MOV_OPCODE;
	jmp->mov_modrm = MOV_MODRM_BYTE;
	jmp->mov_sib = MOV_SIB_BYTE;
	jmp->mov_offset = MOV_OFFSET;
	jmp->mov_addr = (uint32_t)(((uintptr_t)dst) >> 32);
	jmp->ret_opcode = RET_OPCODE;
}

#endif

static size_t subhook_make_trampoline(void *trampoline, void *src) {
	int orig_size = 0;
	int insn_len;
	intptr_t trampoline_addr = (intptr_t)trampoline;
	intptr_t src_addr = (intptr_t)src;
	intptr_t return_dst_addr;

	while (orig_size < JMP_SIZE) {
		int32_t reloc_op_offset = 0;

		insn_len =
			subhook_disasm((void *)(src_addr + orig_size), &reloc_op_offset);

		if (insn_len == 0)
			return 0;

		memcpy(
			(void *)(trampoline_addr + orig_size),
			(void *)(src_addr + orig_size),
			insn_len);

		/* If the operand is a relative address, such as found in calls or
		 * jumps, it needs to be relocated because the original code and the
		 * trampoline reside at different locations in memory.
		 */
		if (reloc_op_offset > 0) {
			/* Calculate how far our trampoline is from the source and change
			 * the address accordingly.
			 */
			int32_t moved_by = (int32_t)(trampoline_addr - src_addr);
			int32_t *op = (int32_t *)(
				trampoline_addr + orig_size + reloc_op_offset);
			*op -= moved_by;
		}

		orig_size += insn_len;
	}

	return_dst_addr = src_addr + orig_size;
	subhook_make_jmp(trampoline, (void *)return_dst_addr);

	return orig_size + JMP_SIZE;
}

SUBHOOK_EXPORT subhook_t SUBHOOK_API subhook_new(void *src, void *dst) {
	subhook_t hook;

	if ((hook = malloc(sizeof(*hook))) == NULL)
		return NULL;

	hook->installed = 0;
	hook->src = src;
	hook->dst = dst;

	if ((hook->code = malloc(JMP_SIZE)) == NULL) {
		free(hook);
		return NULL;
	}

	memcpy(hook->code, hook->src, JMP_SIZE);

	if ((hook->trampoline = calloc(1, MAX_TRAMPOLINE_LEN)) == NULL) {
		free(hook->code);
		free(hook);
		return NULL;
	}

	if (subhook_unprotect(hook->src, JMP_SIZE) == NULL
		|| subhook_unprotect(hook->trampoline, MAX_TRAMPOLINE_LEN) == NULL)
	{
		free(hook->trampoline);
		free(hook->code);
		free(hook);
		return NULL;
	}

	if (subhook_make_trampoline(hook->trampoline, hook->src) == 0) {
		free(hook->trampoline);
		hook->trampoline = NULL;
	}

	return hook;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_free(subhook_t hook) {
	free(hook->trampoline);
	free(hook->code);
	free(hook);
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_install(subhook_t hook) {
	if (hook->installed)
		return -EINVAL;

	subhook_make_jmp(hook->src, hook->dst);
	hook->installed = 1;

	return 0;
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_remove(subhook_t hook) {
	if (!hook->installed)
		return -EINVAL;

	memcpy(hook->src, hook->code, JMP_SIZE);
	hook->installed = 0;

	return 0;
}

#if SUBHOOK_BITS == 32

SUBHOOK_EXPORT void *SUBHOOK_API subhook_read_dst(void *src)  {
	struct subhook_jmp32 *maybe_jmp = (struct subhook_jmp32 *)src;

	if (maybe_jmp->opcode != JMP_OPCODE)
		return NULL;

	return (void *)(maybe_jmp->offset + (uintptr_t)src + JMP_SIZE);
}

#else

SUBHOOK_EXPORT void *SUBHOOK_API subhook_read_dst(void *src)  {
	struct subhook_jmp64 *maybe_jmp = (struct subhook_jmp64 *)src;

	if (maybe_jmp->push_opcode != PUSH_OPCODE)
		return NULL;

	return (void *)(
		maybe_jmp->push_addr & (((uintptr_t)maybe_jmp->mov_addr << 32)));
}

#endif
