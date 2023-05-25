#ifndef __UTF_H__
#define __UTF_H__

#define UTF8_MAX 4
#define UNICODE_MAX 0x10FFFFF

typedef enum
{
	BIG,
	LITTLE
} Endian;

int get_endian();

int utf16tounicode(uint32_t *code_point, uint16_t *utf16);
int utf8tounicode(uint32_t *code_point, uint8_t *utf8);

int unicodetoutf16(uint16_t *utf16, uint32_t code_point);
int unicodetoutf8(uint8_t *utf8, uint32_t code_point);

int utf8toutf16(uint16_t *utf16, uint8_t *utf8);
int utf16toutf8(uint8_t *utf8, uint16_t *utf16);

#endif