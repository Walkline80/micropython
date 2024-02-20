#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/stream.h"

#include <string.h>
#include <stdlib.h>
// #include "esp_log.h"
#include "modfontlib.h"

// static const char *TAG = "modfontlib";
STATIC const mp_obj_type_t fontlib_module_type;

/**
STATIC void fill_rect(uint16_t *buffer, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
					  mp_int_t stride, mp_int_t format, uint32_t color) {
	if (format == FORMAT_MONO_HLSB) {
		unsigned int advance = stride >> 3;

		while (width--) {
			uint8_t *b = &((uint8_t *)buffer)[(x >> 3) + y * advance];
			unsigned int offset = 7 - (x & 7);

			for (unsigned int hh = height; hh; --hh) {
				*b = (*b & ~(0x01 << offset)) | ((color != 0) << offset);
				b += advance;
			}

			++x;
		}
	} else if (format == FORMAT_RGB565) {
		uint16_t *b = &((uint16_t *)buffer)[x + y * stride];

		while (height--) {
			for (unsigned int ww = width; ww; --ww) {
				*b++ = color;
			}

			b += stride - width;
		}
	}
}
 */

/**

STATIC bool has_generated_data(qrcode_QRCODE_obj_t *self) {
	bool result = true;

	if (self->length == 0) {
		result = false;
		mp_printf(&mp_plat_print, "no data generated, call generate() first.\n");
	}

	return result;
}
 */




/*******************************
 * private function definition *
 *******************************/

// static bool __is_ascii(uint16_t char_code)
// {
// 	return (char_code >= ASCII_START) && (char_code <= ASCII_END);
// }

// static bool __is_gb2312(uint16_t char_code)
// {
// 	return (char_code >= GB2312_START) && (char_code <= GB2312_END);
// }

STATIC mp_obj_t __get_unicode_buffer(fontlib_obj_t *self, const mp_stream_p_t *file_stream, uint32_t unicode)
{
	uint32_t char_offset = 0;

	if (IS_ASCII(unicode)) {
		char_offset = self->header->ascii_start + (unicode - ASCII_START) * self->header->data_size;
	} else if (IS_GB2312(unicode)) {
		
	} else {
		// return a place holder dict
	}

	int error_code;
	struct mp_stream_seek_t seek = { .offset = 0, .whence = 0 };
	mp_uint_t res = file_stream->ioctl(self->current_file, MP_STREAM_SEEK, (uintptr_t)&seek, &error_code);
	assert(res != MP_STREAM_ERROR);

	byte readbuf[HEADER_LENGTH];
	mp_uint_t read_count = file_stream->read(self->current_file, readbuf, HEADER_LENGTH, &error_code);

	return mp_const_none;
}
/****************************************/




/***************************
 * class method definition *
 ***************************/

STATIC mp_obj_t fontlib_FONTLIB_get_characters(mp_obj_t self_in, mp_obj_t characters_in)
{
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	mp_obj_t result = mp_obj_new_dict(0);
	const char *chars = mp_obj_str_get_str(characters_in);
	size_t chars_len = strlen(chars);

	mp_printf(&mp_plat_print, "input chars: %s, byte count: %d\n", chars, chars_len);

	uint32_t unicode = 0;
	uint8_t utf8_length = 0, utf8_mask = 0;

	/**
	 * utf8_2_unicode 转换代码来自大佬 wangshujun-tj (https://github.com/wangshujun-tj)
	 */
	for (; *chars; ++chars) {
		uint8_t char_code = *(uint8_t *) chars;

		if (char_code == 9 || char_code == 10 || char_code == 13) {continue;}

		if (char_code >= ASCII_START && char_code <= ASCII_END) {
			utf8_length = 1; unicode = char_code;
		} else if (char_code >= UTF8_START_2 && char_code < UTF8_END_2) {
			utf8_length = 2; utf8_mask = UTF8_MASK_2;
		} else if (char_code >= UTF8_START_3 && char_code < UTF8_END_3) {
			utf8_length = 3; utf8_mask = UTF8_MASK_3;
		} else if (char_code >= UTF8_START_4 && char_code < UTF8_END_4) {
			utf8_length = 4; utf8_mask = UTF8_MASK_4;
		} else if (char_code >= UTF8_START_5 && char_code < UTF8_END_5) {
			utf8_length = 5; utf8_mask = UTF8_MASK_5;
		} else if (char_code > UTF8_START_6) {
			utf8_length = 6; utf8_mask = UTF8_MASK_6;
		}

		if (utf8_length > 1) {
			unicode = (char_code & utf8_mask) << 6 * (utf8_length - 1);

			for (uint8_t i=0; i<utf8_length - 1; i++) {
				++chars;
				char_code = *(uint8_t *) chars;

				if ((char_code & 0xc0) != 0x80) {
					unicode = 0;
					continue;
				} else {
					unicode |= (char_code & 0x3f) << 6 * (utf8_length - 2 - i);
				}
			}
		}

		// mp_obj_new_memoryview buffer = __get_unicode_buffer(self, file_stream, unicode);
		// mp_obj_dict_store(result, mp_obj_new_int(unicode), buffer);
	}

	mp_obj_t open_args[2] = {
		mp_obj_new_str(self->filename, strlen(self->filename)),
		MP_OBJ_NEW_QSTR(MP_QSTR_rb)
	};

	self->current_file = mp_builtin_open(2, open_args, (mp_map_t *) &mp_const_empty_map);
	const mp_stream_p_t *file_stream = mp_get_stream(self->current_file);

	mp_stream_close(self->current_file);

	// with open(self.__font_filename, 'rb') as font_file:
	// 	unicode_list = list(set(ord(char) for char in characters))

	// 	chunk = 30
	// 	for count in range(0, len(unicode_list) // chunk + 1):
	// 		for char in self.__get_character_unicode_buffer(font_file, unicode_list[count * chunk:count * chunk + chunk]):
	// 			result[char[0]] = char[1]

	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(fontlib_FONTLIB_get_characters_obj, fontlib_FONTLIB_get_characters);

STATIC mp_obj_t fontlib_FONTLIB_info(mp_obj_t self_in)
{
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);

	mp_printf(&mp_plat_print, "- FontLib Info (%s):\r\n    file_size: %d\r\n    font_width: %d\r\n    font_height: %d\r\n    data_size: %d\r\n    scan_mode: %d\r\n    byte_order: %d\r\n    format: %d\r\n    ascii_start: %d\r\n    gb2312_start: %d\r\n    characters: %d\r\n",
		self->filename,
		self->header->file_size,
		self->header->font_width,
		self->header->font_height,
		self->header->data_size,
		self->header->scan_mode,
		self->header->byte_order,
		self->header->format,
		self->header->ascii_start,
		self->header->gb2312_start,
		self->header->characters
	);

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_info_obj, fontlib_FONTLIB_info);

STATIC mp_obj_t fontlib_FONTLIB_file_name(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_str(self->filename, strlen(self->filename));
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_file_name_obj, fontlib_FONTLIB_file_name);

STATIC mp_obj_t fontlib_FONTLIB_file_size(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->file_size);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_file_size_obj, fontlib_FONTLIB_file_size);

STATIC mp_obj_t fontlib_FONTLIB_font_width(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->font_width);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_font_width_obj, fontlib_FONTLIB_font_width);

STATIC mp_obj_t fontlib_FONTLIB_font_height(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->font_height);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_font_height_obj, fontlib_FONTLIB_font_height);

STATIC mp_obj_t fontlib_FONTLIB_data_size(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->data_size);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_data_size_obj, fontlib_FONTLIB_data_size);

STATIC mp_obj_t fontlib_FONTLIB_scan_mode(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->scan_mode);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_scan_mode_obj, fontlib_FONTLIB_scan_mode);

STATIC mp_obj_t fontlib_FONTLIB_byte_order(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->byte_order);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_byte_order_obj, fontlib_FONTLIB_byte_order);

STATIC mp_obj_t fontlib_FONTLIB_format(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->format);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_format_obj, fontlib_FONTLIB_format);

STATIC mp_obj_t fontlib_FONTLIB_ascii_start(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->ascii_start);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_ascii_start_obj, fontlib_FONTLIB_ascii_start);

STATIC mp_obj_t fontlib_FONTLIB_gb2312_start(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->gb2312_start);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_gb2312_start_obj, fontlib_FONTLIB_gb2312_start);

STATIC mp_obj_t fontlib_FONTLIB_characters(mp_obj_t self_in) {
	fontlib_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->header->characters);
}
MP_DEFINE_CONST_FUN_OBJ_1(fontlib_FONTLIB_characters_obj, fontlib_FONTLIB_characters);
/****************************************/

/**
 * get qrcode buffered data with mono_hlsb or rgb565 format
 * 
 * code.buffer_data(bytearray, qrcode.FORMAT_MONO_HLSB[, scales])
 * code.buffer_data(bytearray, qrcode.FORMAT_RGB565[, scales, color, bg_color])
STATIC mp_obj_t qrcode_QRCODE_buffer_data(size_t n_args, const mp_obj_t *args) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	if (has_generated_data(self)) {
		mp_buffer_info_t bufinfo;
		mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);
		mp_int_t format = mp_obj_get_int(args[2]);
		mp_int_t scales = 1;
		mp_int_t stride = 0;
		uint16_t color = WHITE;
		uint16_t bg_color = BLACK;

		if (n_args >= 3 && n_args <= 6) {
			scales = mp_obj_get_int(args[3]);
			if (scales < 1) {scales = 1;}
			stride = format == FORMAT_MONO_HLSB ? (self->length * scales + 7) & ~7 : self->length * scales;
		}

		if (n_args >= 5 && n_args <= 6) {
			color = mp_obj_get_int(args[4]);
			if (format == FORMAT_MONO_HLSB) {
				color = 1;
			} else {
				color = RGB2BGR_565(color);
			}
		}

		if (n_args == 6) {bg_color = RGB2BGR_565(mp_obj_get_int(args[5]));}

		for (int y = 0; y < self->length; y++) {
			for (int x = 0; x < self->length; x++) {
				if (qrcodegen_getModule(self->buffer, x, y)) {
					fill_rect(bufinfo.buf, x * scales, y * scales, scales, scales, stride, format, color);
				} else {
					if (format == FORMAT_RGB565) {
						fill_rect(bufinfo.buf, x * scales, y * scales, scales, scales, stride, format, bg_color);
					}
				}
			}
		}
	}

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qrcode_QRCODE_buffer_data_obj, 3, 6, qrcode_QRCODE_buffer_data);
 **/

/**
 * get qrcode raw data with tuple format
 * 
 * code.raw_data()
STATIC mp_obj_t qrcode_QRCODE_raw_data(mp_obj_t self_in) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (has_generated_data(self)) {
		mp_int_t size = self->length;
		mp_obj_t raw_data[size];
		mp_obj_t row[size];

		for (int y = 0; y < size; y++) {
			for (int x = size - 1; x >= 0; x--) {
				row[x] = mp_obj_new_int(qrcodegen_getModule(self->buffer, x, y) ? 1 : 0);
			}

			raw_data[y] = mp_obj_new_tuple(size, row);
		}

		return mp_obj_new_tuple(size, raw_data);
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_raw_data_obj, qrcode_QRCODE_raw_data);
 **/

/**
 * print qrcode in console
 * 
 * code.print()
STATIC mp_obj_t qrcode_QRCODE_print(mp_obj_t self_in) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	if (has_generated_data(self)) {
		mp_int_t size = self->length;
		mp_int_t border = 2;
		unsigned char num = 0;

		for (int y = -border; y < size + border; y+=2) {
			for (int x = -border; x < size + border; x+=2) {
				num = 0;
				if (qrcodegen_getModule(self->buffer, x, y)) {num |= 1 << 0;}
				if ((x < size + border) && qrcodegen_getModule(self->buffer, x+1, y)) {num |= 1 << 1;}
				if ((y < size + border) && qrcodegen_getModule(self->buffer, x, y+1)) {num |= 1 << 2;}
				if ((x < size + border) && (y < size + border) && qrcodegen_getModule(self->buffer, x+1, y+1)) {num |= 1 << 3;}
				printf("%s", lt[num]);
			}
			printf("\n");
		}
		printf("\n");
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_print_obj, qrcode_QRCODE_print);
 **/

/**
STATIC mp_obj_t qrcode_QRCODE_generate(mp_obj_t self_in, mp_obj_t text_in) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);
	const char *text = mp_obj_str_get_str(text_in);
	uint8_t *tempbuf;

	tempbuf = calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(self->max_version));

	if (!tempbuf) {
		mp_raise_msg_varg(
			&mp_type_OSError,
			MP_ERROR_TEXT("out of memory, %ld bytes required."),
			(long) qrcodegen_BUFFER_LEN_FOR_VERSION(self->max_version)
		);
	}

	ESP_LOGI(TAG, "Encoding below text with ECC LVL %d & QR Code Version %d",
			 self->ecc_level, self->max_version);
	ESP_LOGI(TAG, "%s", text);

	// Make the QR Code symbol
	bool ok = qrcodegen_encodeText(
		text,
		tempbuf,
		self->buffer,
		self->ecc_level,
		qrcodegen_VERSION_MIN,
		self->max_version,
		qrcodegen_Mask_AUTO,
		true
	);

	free(tempbuf);

	if (ok) {
		self->length = qrcodegen_getSize(self->buffer);
		self->version = (self->length - 17) / 4;
		self->buffer_size = (self->length * self->length - 1) / 8;

		return mp_obj_new_bool(true);
	}

	return mp_obj_new_bool(false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(qrcode_QRCODE_generate_obj, qrcode_QRCODE_generate);
 **/

/**
 * get length param
 * 
 * code.length()
STATIC mp_obj_t qrcode_QRCODE_length(mp_obj_t self_in) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	return mp_obj_new_int(self->length);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_length_obj, qrcode_QRCODE_length);
 **/

/**
 * get version param
 * 
 * code.version()
STATIC mp_obj_t qrcode_QRCODE_version(mp_obj_t self_in) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	return mp_obj_new_int(self->version);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_version_obj, qrcode_QRCODE_version);
 **/

/**
 * get/set ecc level param
 * 
 * code.ecc_level()
 * code.ecc_level(3)
STATIC mp_obj_t qrcode_QRCODE_ecc_level(size_t n_args, const mp_obj_t *args) {
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	if (n_args == 1) {
		return mp_obj_new_int(self->ecc_level);
	} else {
		if (mp_obj_is_integer(args[1])) {
			mp_int_t ecc_level = mp_obj_get_int(args[1]);

			if (ecc_level >= ESP_QRCODE_ECC_LOW && ecc_level <= ESP_QRCODE_ECC_HIGH) {
				self->ecc_level = ecc_level;
			}
		} else {
			mp_raise_ValueError(MP_ERROR_TEXT("invalid arguments"));
		}
	}

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qrcode_QRCODE_ecc_level_obj, 1, 2, qrcode_QRCODE_ecc_level);
**/




/*******************
 * type definition *
 *******************/

/**
static void fontlib_module_type_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
	(void) kind;
	mp_printf(print, "- FontLib Info");
}
*/

mp_obj_t fontlib_FONTLIB_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
	mp_arg_check_num(n_args, n_kw, 1, 1, false);

	fontlib_obj_t *self = m_new_obj(fontlib_obj_t);

	self->base.type	= &fontlib_module_type;
	self->filename	= mp_obj_str_get_str(args[0]);

	mp_obj_t open_args[2] = {
		mp_obj_new_str(self->filename, strlen(self->filename)),
		MP_OBJ_NEW_QSTR(MP_QSTR_rb)
	};

	self->current_file = mp_builtin_open(2, open_args, (mp_map_t *) &mp_const_empty_map);

	const mp_stream_p_t *file_stream = mp_get_stream(self->current_file);
	int error_code;
	byte readbuf[HEADER_LENGTH];
	mp_uint_t read_count = file_stream->read(self->current_file, readbuf, HEADER_LENGTH, &error_code);

	if (read_count == 0 || read_count == MP_STREAM_ERROR) {
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Read font file header info error."));
		// mp_raise_msg_varg(&mp_type_OSError,
        //             MP_ERROR_TEXT("Cannot : %d\n"), error);
	}

	fontlib_header_t *header = m_new_obj(fontlib_header_t);

	header->identify[0]			= readbuf[0];
	header->identify[1]			= readbuf[1];
	header->identify[2]			= readbuf[2];
	header->identify[3]			= readbuf[3];
	header->identify[4]			= '\0';

	if (strcmp(header->identify, "FMUX") != 0 || strcmp(header->identify, "FMUY") != 0) {
		mp_stream_close(self->current_file);
		mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Font file format error!"));
		exit(1);
	}

	header->file_size			= readbuf[4] + 
								  (readbuf[5] << 8) + 
								  (readbuf[6] << 16) + 
								  (readbuf[7] << 24);
	header->font_width			= readbuf[8];
	header->font_height			= readbuf[9];
	header->characters			= readbuf[10] + (readbuf[11] << 8);
	header->has_index_table		= readbuf[12];
	header->scan_mode			= readbuf[13];
	header->byte_order			= readbuf[14];
	header->ascii_start			= readbuf[15] + 
								  (readbuf[16] << 8) + 
								  (readbuf[17] << 16) + 
								  (readbuf[18] << 24);
	header->gb2312_start		= readbuf[19] + 
								  (readbuf[20] << 8) + 
								  (readbuf[21] << 16) + 
								  (readbuf[22] << 24);
	header->data_size			= ((header->font_width - 1) / 8 + 1) * header->font_height;
	header->index_table_address	= header->has_index_table ? HEADER_LENGTH : 0;
	header->format				= MONO_VLSB;

	if (header->scan_mode == SCAN_MODE_HORIZONTAL) {
		header->format = header->byte_order == BYTE_ORDER_MSB ? MONO_HMSB : MONO_HLSB;
	}

	mp_stream_close(self->current_file);

	self->header = header;




	// self->buffer_size = 0;
	// self->buffer      = calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(args[ARG_max_version].u_int));

	// if (!self->buffer) {
	// 	mp_raise_msg_varg(
	// 		&mp_type_OSError,
	// 		MP_ERROR_TEXT("out of memory, %ld bytes required."),
	// 		(long) qrcodegen_BUFFER_LEN_FOR_VERSION(args[ARG_max_version].u_int)
	// 	);
	// }

	return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t fontlib_FONTLIB_locals_dict_table[] = {
	{MP_ROM_QSTR(MP_QSTR_file_name),		MP_ROM_PTR(&fontlib_FONTLIB_file_name_obj)},
	{MP_ROM_QSTR(MP_QSTR_file_size),		MP_ROM_PTR(&fontlib_FONTLIB_file_size_obj)},
	{MP_ROM_QSTR(MP_QSTR_font_width),		MP_ROM_PTR(&fontlib_FONTLIB_font_width_obj)},
	{MP_ROM_QSTR(MP_QSTR_font_height),		MP_ROM_PTR(&fontlib_FONTLIB_font_height_obj)},
	{MP_ROM_QSTR(MP_QSTR_data_size),		MP_ROM_PTR(&fontlib_FONTLIB_data_size_obj)},
	{MP_ROM_QSTR(MP_QSTR_scan_mode),		MP_ROM_PTR(&fontlib_FONTLIB_scan_mode_obj)},
	{MP_ROM_QSTR(MP_QSTR_byte_order),		MP_ROM_PTR(&fontlib_FONTLIB_byte_order_obj)},
	{MP_ROM_QSTR(MP_QSTR_format),			MP_ROM_PTR(&fontlib_FONTLIB_format_obj)},
	{MP_ROM_QSTR(MP_QSTR_ascii_start),		MP_ROM_PTR(&fontlib_FONTLIB_ascii_start_obj)},
	{MP_ROM_QSTR(MP_QSTR_gb2312_start),		MP_ROM_PTR(&fontlib_FONTLIB_gb2312_start_obj)},
	{MP_ROM_QSTR(MP_QSTR_characters),		MP_ROM_PTR(&fontlib_FONTLIB_characters_obj)},

	{MP_ROM_QSTR(MP_QSTR_info),				MP_ROM_PTR(&fontlib_FONTLIB_info_obj)},
	{MP_ROM_QSTR(MP_QSTR_get_characters),	MP_ROM_PTR(&fontlib_FONTLIB_get_characters_obj)},
};
STATIC MP_DEFINE_CONST_DICT(fontlib_FONTLIB_locals_dict, fontlib_FONTLIB_locals_dict_table);

STATIC MP_DEFINE_CONST_OBJ_TYPE(
	fontlib_module_type,
	MP_QSTR_FontLib,
	MP_TYPE_FLAG_NONE,
	// print, fontlib_module_type_print,
	make_new, fontlib_FONTLIB_make_new,
	locals_dict, &fontlib_FONTLIB_locals_dict
);




/*********************
 * module definition *
 *********************/

STATIC const mp_rom_map_elem_t fontlib_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR___name__),		MP_ROM_QSTR(MP_QSTR_fontlib)},
	{MP_ROM_QSTR(MP_QSTR_FontLib),		(mp_obj_t) &fontlib_module_type},

	/* ecc levels*/
	// {MP_ROM_QSTR(MP_QSTR_ECC_LOW),          MP_ROM_INT(ECC_LOW)},
	// {MP_ROM_QSTR(MP_QSTR_ECC_MED),          MP_ROM_INT(ECC_MED)},
	// {MP_ROM_QSTR(MP_QSTR_ECC_QUART),        MP_ROM_INT(ECC_QUART)},
	// {MP_ROM_QSTR(MP_QSTR_ECC_HIGH),         MP_ROM_INT(ECC_HIGH)},

	/* versions */
	// {MP_ROM_QSTR(MP_QSTR_VERSION_MIN),      MP_ROM_INT(VERSION_MIN)},
	// {MP_ROM_QSTR(MP_QSTR_VERSION_MAX),      MP_ROM_INT(VERSION_MAX)},

	/* format*/
	// {MP_ROM_QSTR(MP_QSTR_FORMAT_MONO_HLSB), MP_ROM_INT(FORMAT_MONO_HLSB)},
	// {MP_ROM_QSTR(MP_QSTR_FORMAT_RGB565),    MP_ROM_INT(FORMAT_RGB565)},
};

STATIC MP_DEFINE_CONST_DICT(fontlib_module_globals, fontlib_module_globals_table);

const mp_obj_module_t fontlib_user_cmodule = {
	.base	 = {&mp_type_module},
	.globals = (mp_obj_dict_t *) &fontlib_module_globals,
};

#if MODULE_FONTLIB_ENABLED
MP_REGISTER_MODULE(MP_QSTR_fontlib, fontlib_user_cmodule);
#endif
