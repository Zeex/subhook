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

#ifndef SUBHOOK_H
#define SUBHOOK_H

#include <stddef.h>

#if defined _M_IX86 || defined __i386__
	#define SUBHOOK_X86
#else
	#error Unsupported architecture
#endif

#if defined __linux__
	#define SUBHOOK_LINUX
#elif defined _WIN32
	#define SUBHOOK_WINDOWS
#else
	#error Unsupported operating system
#endif

#if !defined SUHOOK_EXTERN
	#if defined __cplusplus
		#define SUBHOOK_EXTERN extern "C"
	#else 
		#define SUBHOOK_EXTERN extern
	#endif
#endif

#if defined SUBHOOK_STATIC
	#define SUBHOOK_API
	#define SUBHOOK_EXPORT SUBHOOK_EXTERN
#endif

#if !defined SUBHOOK_API
	#if defined SUBHOOK_WINDOWS
		#define SUBHOOK_API __cdecl
	#elif defined SUBHOOK_LINUX
		#define SUBHOOK_API __attribute__((cdecl))
	#endif
#endif

#if !defined SUBHOOK_EXPORT
	#if defined SUBHOOK_WINDOWS
		#if defined SUBHOOK_IMPLEMENTATION
			#define SUBHOOK_EXPORT SUBHOOK_EXTERN __declspec(dllexport)
		#else
			#define SUBHOOK_EXPORT SUBHOOK_EXTERN __declspec(dllimport)
		#endif
	#elif defined SUBHOOK_LINUX
		#if defined SUBHOOK_IMPLEMENTATION
			#define SUBHOOK_EXPORT SUBHOOK_EXTERN __attribute__((visibility("default")))
		#else
			#define SUBHOOK_EXPORT SUBHOOK_EXTERN
		#endif
	#endif
#endif

#define SUBHOOK_FLAG_INSTALLED 0x01

SUBHOOK_EXPORT struct subhook *SUBHOOK_API subhook_new();
SUBHOOK_EXPORT void SUBHOOK_API subhook_free(struct subhook *hook);

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_source(struct subhook *hook, void *src);
SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_source(struct subhook *hook);

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_destination(struct subhook *hook, void *dst);
SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_destination(struct subhook *hook);

/* These return 0 on failure and 1 on success. */
SUBHOOK_EXPORT int SUBHOOK_API subhook_install(struct subhook *hook);
SUBHOOK_EXPORT int SUBHOOK_API subhook_remove(struct subhook *hook);

SUBHOOK_EXPORT int SUBHOOK_API subhook_get_flags(struct subhook *hook);
SUBHOOK_EXPORT void SUBHOOK_API subhook_set_flags(struct subhook *hook, int flags);

/* Sets read+write+execute permissions for memory region. */
SUBHOOK_EXPORT void *SUBHOOK_API subhook_unprotect(void *address, size_t size);

static int subhook_is_installed(struct subhook *hook) {
	return (subhook_get_flags(hook) & SUBHOOK_FLAG_INSTALLED) != 0;
}

/* Reads hook destination address from code.
 *
 * This is useful when you don't know the address or want to check
 * whether src has been hooked with subhook.
 */
SUBHOOK_EXPORT void *SUBHOOK_API subhook_read_destination(void *src);

#define SUBHOOK_INSTALL_HOOK(hook, src, dest) \
	do {\
		subhook_set_source(hook, src);\
		subhook_set_destination(hook, dest);\
		subhook_install(hook);\
	} while (0);

#ifdef __cplusplus

class SubHook {
public:
	SubHook() {
		hook_ = subhook_new();
		subhook_set_source(hook_, 0);
		subhook_set_destination(hook_, 0);
	}

	SubHook(void *src, void *dst) {
		hook_ = subhook_new();
		subhook_set_source(hook_, src);
		subhook_set_destination(hook_, dst);
	}

	~SubHook() {
		if (installed_) {
			subhook_remove(hook_);
			subhook_free(hook_);
		}
	}

	bool Install() {
		if (!installed_) {
			subhook_install(hook_);
			installed_ = true;
			return true;
		}
		return false;
	}

	bool Install(void *src, void *dst) {
		if (!installed_) {
			subhook_set_source(hook_, src);
			subhook_set_destination(hook_, dst);
			subhook_install(hook_);
			installed_ = true;
			return true;
		}
		return false;
	}

	bool Remove() {
		if (installed_) {
			subhook_remove(hook_);
			installed_ = false;
			return true;
		}
		return false;
	}

	bool IsInstalled() const {
		return installed_;
	}

	class ScopedRemove {
	public:
		ScopedRemove(SubHook *hook)
			: hook_(hook)
			, removed_(hook_->Remove())
		{
		}

		~ScopedRemove() {
			if (removed_) {
				hook_->Install();
			}
		}

	private:
		SubHook *hook_;
		bool removed_;
	};

	class ScopedInstall {
	public:
		ScopedInstall(SubHook *hook)
			: hook_(hook)
			, installed_(hook_->Install())
		{
		}

		~ScopedInstall() {
			if (installed_) {
				hook_->Remove();
			}
		}

	private:
		SubHook *hook_;
		bool installed_;
	};

private:
	SubHook(const SubHook &);
	void operator=(const SubHook &);

private:
	subhook *hook_;
	bool installed_;
};

#endif

#endif /* SUBHOOK_H */
