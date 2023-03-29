#include <stdio.h>
#include "../mujs.h"

static void jsB_print(js_State *J)
{
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stdout);
	}
	putchar('\n');
	js_pushundefined(J);
}

int main (int argc, char *argv[]) {

	js_State *J;
	J = js_newstate(NULL, NULL, 0);

	js_newcfunction(J, jsB_print, "print", 0);
	js_setglobal(J, "print");

	js_dostring(J, "function hello(greet){debugger;print('helo, ', greet);};hello('world')");

	js_freestate(J);

	return 0;
}