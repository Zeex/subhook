#include <subhook.h>

extern void foo();
extern void foo_hook();

subhook_t hfoo;

int main() {
	hfoo = subhook_new((void *)foo, (void *)foo_hook);
	subhook_install(hfoo);
	foo();
	subhook_remove(hfoo);
	foo();
	subhook_install(hfoo);
	foo();
	subhook_remove(hfoo);
	foo();
	subhook_free(hfoo);
}
