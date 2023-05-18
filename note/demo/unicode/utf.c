/* gcc -g -Wall utf.c -o utf
主要实现测试大小端，unicode code point转化为utf-16，utf-8
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UTF8_MAX 4
#define UNICODE_MAX 0x10FFFFF

typedef union
{
	uint16_t i16;
	uint8_t i8[2];
} endian;

int unicodetoutf16(uint16_t *utf16, uint32_t code_point)
{
	// 0 - 0xD7FF, 0xE000 - 0xFFFF
	if((code_point > 0xD7FF && code_point < 0xE000) || code_point > 0x10FFFF)
	{
		return -1;
	}
	// Basic Multilingual Plane
	if(code_point < 0x10000)
	{
		*utf16 = (code_point & 0xff) | ((code_point >> 8 & 0xff) << 8);
		return 1;
	}
	if(code_point < 0x110000)
	{
		// 0xD800 - 0xDBFF
		// 0xDC00 - 0xDFFF
		uint32_t buf = code_point - 0x10000;
		utf16[0] = 0xD800 | ((buf >> 10) & 0x03FF);
		utf16[1] = 0xDC00 | (buf & 0x03FF);
		return 2;
	}
	return -1;
}

int unicodetoutf8(uint8_t *utf8, uint32_t code_point)
{
	int size = 0;
// 	000800 - 00D7FF
// 	00E000 - 00FFFF
	if(code_point > 0x10FFFF
		|| (code_point > 0xD7FF && code_point < 0xE000))
		return -1;
	if(code_point < 0x80)
	{
		size = 1;
		*utf8 =code_point;
		return size;
	}
	if(code_point < 0x800)
	{
		size = 2;
		// 110y yyyy 10zz zzzz
		utf8[0] = 0b11000000 | (code_point >> 6);
		utf8[1] = 0b11000000 | (code_point & 0x003f);
		return size;
	}
	if(code_point < 0x10000) 
	{
		size = 3;
		// 1110 yyyy 10xx xxxx 10zz zzzz
		utf8[2] = 0b10000000 | (code_point & 0x003f);
		utf8[1] = 0b10000000 | ((code_point >> 6) & 0x003f);
		utf8[0] = 0b11100000 | (code_point >> 12);
		return size;
	}
	if(code_point < 0x110000)
	{
		size = 4;
		// 1111 0www 10xx xxxx 10yy yyyy 10zz zzzz
		utf8[3] = 0b10000000 | (code_point & 0x003f);
		utf8[2] = 0b10000000 | ((code_point >> 6) & 0x003f);
		utf8[1] = 0b10000000 | ((code_point >> 12) & 0x003f);
		utf8[0] = 0b11110000 | (code_point >> 18);
		return size;
	}
	return -1;
}

int main()
{
	// endian check
	endian e;
	e.i16 = 0xFEFF;
	if(e.i8[0] == 0xFE) 
	{
		printf("Big Endian: e.i8[0] = 0x%x\n", e.i8[0]);
	}
	else
	{
		printf("Little Endian: e.i8[0] = 0x%x\n", e.i8[0]);
	}

	uint8_t *utf8 = malloc(sizeof(uint8_t)*UTF8_MAX);
	uint16_t *utf16 = malloc(sizeof(uint16_t)*2);
	int size = 0;

	uint32_t cp[] = {0x35, 0x370, 0xFE30, 0x1F60A};
	for(int i = 0; i < (sizeof(cp) / sizeof(cp[0])); i++)
	{
		size = unicodetoutf8(utf8, cp[i]);
		printf("utf8 size: %d, 0x%x 0x%x 0x%x 0x%x\n", size, utf8[0], utf8[1], utf8[2], utf8[3]);

		size = unicodetoutf16(utf16, cp[i]);
		printf("utf16 size: %d, 0x%x 0x%x\n", size, utf16[0], utf16[1]);
	}
	return 0;
}