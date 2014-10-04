#include <subhook.h>

extern foo();
extern foo_hook();

subhook_t hfoo;

int main() {
	hfoo = subhook_new();
	subhook_set_src(hfoo, (void*)foo);
	subhook_set_dst(hfoo, (void*)foo_hook);
	foo();
	subhook_install(hfoo);
	foo();
	subhook_remove(hfoo);
	foo();
	subhook_free(hfoo);
}