/* Copyright (c) 2012 Zeex
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

#include <stdlib.h>

#include "subhook.h"
#include "subhook_private.h"

SUBHOOK_EXPORT struct subhook *SUBHOOK_API subhook_new() {
	return (struct subhook *)calloc(1, sizeof(struct subhook));
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_free(struct subhook *hook) {
	free(hook->arch);
	free(hook);
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_source(struct subhook *hook) {
	return hook->src;
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_destination(struct subhook *hook) {
	return hook->dst;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_source(struct subhook *hook, void *src) {
	hook->src = src;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_destination(struct subhook *hook, void *dst) {
	hook->dst = dst;
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_get_flags(struct subhook *hook) {
	return hook->flags;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_flags(struct subhook *hook, int flags) {
	hook->flags = flags;
}

#if defined SUBHOOK_WINDOWS
	#include "subhook_windows.c"
#elif defined SUBHOOK_LINUX
	#include "subhook_linux.c"
#endif

#if defined SUBHOOK_X86
	#include "subhook_x86.c"
#endif
