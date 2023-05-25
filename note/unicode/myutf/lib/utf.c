/* gcc -g -Wall utf.c -o utf
主要实现测试大小端，unicode code point转化为utf-16，utf-8
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utf.h"

typedef union
{
	uint16_t i16;
	uint8_t i8[2];
} endian_container;

int get_endian()
{
	// endian check
	endian_container e;
	e.i16 = 0xFEFF;
	if(e.i8[0] == 0xFE) 
	{
		return BIG;
	}
	else
	{
		return LITTLE;
	}
	return -1;
}

// 检查code point是否在正常范围值
int isvalid(uint32_t cp)
{
	if(cp < 0)
		return 0; /* negative value */

	if(cp >= 0xD800 && cp <= 0xDFFF)
		return 0; /* surrogate pair range */

	if(cp >= 0xFDD0 && cp <= 0xFDEF)
		return 0; /* non-character range */

	if((cp & 0xFFFE) == 0xFFFE)
		return 0; /* non-character at end of plane */

	if(cp > 0x10FFFF)
		return 0; /* too large, thanks to UTF-16 */

	return 1;
}

int utf16tounicode(uint32_t *code_point, uint16_t *utf16)
{
	// 0xD800 - 0xDBFF high surrogate pair
	// 0xDC00 - 0xDFFF
	if(*utf16 > 0xD7FF && *utf16 < 0xDC00)
	{
		uint32_t high = *utf16 & 0x3ff;
		uint32_t low = *(utf16+1) & 0x3ff;
		uint32_t buf = (high << 10) | low;
		*code_point = buf + 0x10000;
		return 2;
	}
	*code_point = *utf16;
	return 1;
}

int utf8tounicode(uint32_t *code_point, uint8_t *utf8)
{
	// check the first bit of the first byte, if 0 then ascii char
	int ascii_flag = *utf8 & 0x80;
	// 0aaa aaaa
	if(!ascii_flag) 
	{
		*code_point = *utf8;
		return 1;
	}
	// 1111 0www | 10xx xxxx | 10yy yyyy | 10zz zzzz
	if((*utf8 & 0xF0) == 0xF0)
	{
		uint32_t high = *utf8 & 0x07;
		uint32_t high_mid = *(utf8+1) & 0x3f;
		uint32_t low_mid = *(utf8+2) & 0x3f;
		uint32_t low = *(utf8+3) & 0x3f;
		*code_point = (high << 18) | (high_mid << 12) | (low_mid << 6) | low;
		return 4;
	}
	// 1110 xxxx 10yy yyyy 10zz zzzz
	if((*utf8 & 0xE0) == 0xE0)
	{
		uint32_t high = *utf8 & 0x0f;
		uint32_t mid = *(utf8+1) & 0x3f;
		uint32_t low = *(utf8+2) & 0x3f;
		*code_point = (high << 12) | (mid << 6) | low;
		return 3;
	}
	// 110y yyyy 10yy yyyy
	if((*utf8 & 0xC0) == 0xC0)
	{
		uint32_t high = *utf8 & 0x1f;
		uint32_t low = *(utf8 + 1) & 0x3f;
		*code_point = (high << 6) | low;
		return 2;
	}
	*code_point = 0xFFFE; // invalid char
	return 0;
}

int unicodetoutf16(uint16_t *utf16, uint32_t code_point)
{
	if(!isvalid(code_point))
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
	if(!isvalid(code_point))
	{
		return -1;
	}
	int size = 0;
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
		utf8[1] = 0b10000000 | (code_point & 0x003f);
		utf8[0] = 0b11000000 | (code_point >> 6);
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

int utf8toutf16(uint16_t *utf16, uint8_t *utf8)
{
	uint32_t code_point;
	utf8tounicode(&code_point, utf8);
	return unicodetoutf16(utf16, code_point);
}

int utf16toutf8(uint8_t *utf8, uint16_t *utf16)
{
	uint32_t code_point;
	utf16tounicode(&code_point, utf16);
	return unicodetoutf8(utf8, code_point);
}

// int main()
// {

// 	uint8_t *utf8 = malloc(sizeof(uint8_t)*UTF8_MAX);
// 	uint16_t *utf16 = malloc(sizeof(uint16_t)*2);
// 	int size = 0;

// 	uint32_t cp[] = {0x35, 0x370, 0xFE30, 0xFEFF, 0x1F60A};
// 	for(int i = 0; i < (sizeof(cp) / sizeof(cp[0])); i++)
// 	{
// 		size = unicodetoutf8(utf8, cp[i]);
// 		printf("utf8 size: %d, 0x%x 0x%x 0x%x 0x%x\n", size, utf8[0], utf8[1], utf8[2], utf8[3]);

// 		size = unicodetoutf16(utf16, cp[i]);
// 		printf("utf16 size: %d, 0x%x 0x%x\n", size, utf16[0], utf16[1]);
// 	}

// 	uint32_t code_point = 0;
// 	*utf16 = 0xD950;
// 	*(utf16+1) = 0xDF21;
// 	size = utf16tounicode(&code_point, utf16);
// 	printf("0x64321 utf16 to unicode: 0x%x\n", code_point);
// 	*utf8 = 0x35;
// 	*(utf8+1) = 0x9f;
// 	*(utf8+2) = 0x98;
// 	*(utf8+3) = 0x80;
// 	size = utf8tounicode(&code_point, utf8);
// 	printf("0x1F600 utf8 to unicode: 0x%x, len: %d\n", code_point, size);
// 	return 0;
// }