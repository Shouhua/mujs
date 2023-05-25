/*
* 读取json文件，在js中操作后返回c
*/
#include <stdio.h>
#include <stdlib.h>
#include "mujs.h"

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
static const char *console_js =
	"var console = { log: print, debug: print, warn: print, error: print };"
;

int main (int argc, char *argv[]) {
	if(argc != 2) {
		fprintf(stderr, "USAGE: demo2 json_file");
	}
	FILE *f = fopen(argv[1], "r");
	fseek(f, 0, SEEK_END);
	unsigned int len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *menu = (char *)malloc(len+1);
	fread(menu, 1, len, f);
	menu[len] = 0;

	js_State *J = js_newstate(NULL, NULL, 0);
	js_newcfunction(J, jsB_print, "print", 0);
	js_setglobal(J, "print");
	js_dostring(J, console_js);
	js_dofile(J, "m.js");

	js_getglobal(J, "handle");
	js_pushnull(J);
	js_pushstring(J, menu);
	js_pcall(J, 1);

	const char *result = js_tostring(J, -1);
	printf("%s\n", result);

	fclose(f);
	js_freestate(J);
	return 0;
}