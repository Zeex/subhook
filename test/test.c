#include <stdio.h>
#include <subhook.h>

subhook_t hfoo, hbar;

void foo() {
	printf("foo() called\n");
}

void foo_hook() {
	printf("foo_hook() called\n");
}

void test_foo() {
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

void bar() {
	printf("bar() called\n");
}

void bar_hook() {
	printf("bar_hook() called\n");
	subhook_remove(hbar);
	bar();
	subhook_install(hbar);
}

void test_bar() {
	hbar = subhook_new();
	subhook_set_src(hbar, (void*)bar);
	subhook_set_dst(hbar, (void*)bar_hook);
	bar();
	subhook_install(hbar);
	bar();
	subhook_remove(hbar);
	bar();
	subhook_free(hbar);
}

int main() {
	test_foo();
	test_bar();
}
