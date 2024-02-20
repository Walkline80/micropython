#pragma once

#include <stdio.h>

#define ECC_LOW   ESP_QRCODE_ECC_LOW   	/* QR Code Error Tolerance of 7% */
#define ECC_MED	  ESP_QRCODE_ECC_MED    /* QR Code Error Tolerance of 15% */
#define ECC_QUART ESP_QRCODE_ECC_QUART  /* QR Code Error Tolerance of 25% */
#define ECC_HIGH  ESP_QRCODE_ECC_HIGH   /* QR Code Error Tolerance of 30% */

#define VERSION_MIN qrcodegen_VERSION_MIN   /* 1 */
#define VERSION_MAX qrcodegen_VERSION_MAX   /* 40 */

#define	BLACK   0x0000
#define WHITE   0xFFFF

#define RGB2BGR_565(color)  ((color & 0xF800) >> 11) | (color & 0x07E0) | ((color & 0x001F) << 11)



#define HEADER_LENGTH	25

#define ASCII_START		0x20
#define ASCII_END		0x7f
#define GB2312_START	0x80
#define GB2312_END		0xffef

#define IS_ASCII(code)	(code) >= ASCII_START && (code) <= ASCII_END
#define IS_GB2312(code)	(code) >= GB2312_START && (code) <= GB2312_END

/**
 * @brief utf-8字节长度范围
 */
#define UTF8_START_2	0xc0
#define UTF8_END_2		0xe0
#define UTF8_MASK_2		0x1f
#define UTF8_START_3	0xe0
#define UTF8_END_3		0xf0
#define UTF8_MASK_3		0x0f
#define UTF8_START_4	0xf0
#define UTF8_END_4		0xf8
#define UTF8_MASK_4		0x07
#define UTF8_START_5	0xf8
#define UTF8_END_5		0xfc
#define UTF8_MASK_5		0x03
#define UTF8_START_6	0xfc
#define UTF8_MASK_6		0x01

enum {
	SCAN_MODE_HORIZONTAL,
	SCAN_MODE_VERTICAL
};

enum {
	BYTE_ORDER_LSB,
	BYTE_ORDER_MSB
};

typedef enum {
	MONO_VLSB = 0,
	MONO_HLSB = 3,
	MONO_HMSB = 4
} fb_format_t;

// typedef struct _fontlib_header_t{
// 	char		identify[5];
// 	uint32_t	file_size;
// 	uint8_t		font_width;
// 	uint8_t		font_height;
// 	uint16_t	data_size;
// 	uint16_t	characters;
// 	uint8_t		has_index_table;
// 	uint32_t	index_table_address;
// 	uint8_t		scan_mode;
// 	uint8_t		byte_order;
// 	fb_format_t	format;
// 	uint32_t	ascii_start;
// 	uint32_t	gb2312_start;
// 	char		reserved[2];
// } fontlib_header_t;

typedef struct _fontlib_header_t{
	char		identify[5];
	uint32_t	file_size;
	uint8_t		font_width;
	uint8_t		font_height;
	uint16_t	data_size;
	uint16_t	characters;
	uint8_t		has_index_table;
	uint32_t	index_table_address;
	uint8_t		scan_mode;
	uint8_t		byte_order;
	fb_format_t	format;
	uint32_t	ascii_start;
	uint32_t	gb2312_start;
	char		reserved[2];
} fontlib_header_t;

typedef struct _fontlib_obj_t{
	mp_obj_base_t		base;
	const char			*filename;
    fontlib_header_t	*header;
	FILE				*current_file;
} fontlib_obj_t;
