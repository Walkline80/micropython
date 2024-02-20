#include "py/obj.h"
#include "py/runtime.h"

#include <string.h>
#include <stdlib.h>
#include "modqrcode.h"

STATIC const mp_obj_type_t qrcode_QRCODE_type;

static const char *lt[] = {
	/* 0 */ "  ",
	/* 1 */ "\u2580 ",
	/* 2 */ " \u2580",
	/* 3 */ "\u2580\u2580",
	/* 4 */ "\u2584 ",
	/* 5 */ "\u2588 ",
	/* 6 */ "\u2584\u2580",
	/* 7 */ "\u2588\u2580",
	/* 8 */ " \u2584",
	/* 9 */ "\u2580\u2584",
	/* 10 */ " \u2588",
	/* 11 */ "\u2580\u2588",
	/* 12 */ "\u2584\u2584",
	/* 13 */ "\u2588\u2584",
	/* 14 */ "\u2584\u2588",
	/* 15 */ "\u2588\u2588",
};

STATIC void fill_rect(uint16_t *buffer, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
					  mp_int_t stride, mp_int_t format, uint32_t color)
{
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

STATIC bool has_generated_data(qrcode_QRCODE_obj_t *self)
{
	bool result = true;

	if (self->length == 0) {
		result = false;
		mp_printf(&mp_plat_print, "no data generated, call generate() first.\n");
	}

	return result;
}
/****************************************/


/******************************
 * region QRCODE class method *
 ******************************/

/**
 * get qrcode buffered data with mono_hlsb or rgb565 format
 * 
 * code.buffer_data(bytearray, qrcode.FORMAT_MONO_HLSB[, scales])
 * code.buffer_data(bytearray, qrcode.FORMAT_RGB565[, scales, color, bg_color])
 **/
STATIC mp_obj_t qrcode_QRCODE_buffer_data(size_t n_args, const mp_obj_t *args)
{
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

/**
 * get qrcode raw data with tuple format
 * 
 * code.raw_data()
 **/
STATIC mp_obj_t qrcode_QRCODE_raw_data(mp_obj_t self_in)
{
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

/**
 * print qrcode in console
 * 
 * code.print()
 **/
STATIC mp_obj_t qrcode_QRCODE_print(mp_obj_t self_in)
{
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

				mp_printf(&mp_plat_print, "%s", lt[num]);
			}

			mp_printf(&mp_plat_print, "\n");
		}

		mp_printf(&mp_plat_print, "\n");
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_print_obj, qrcode_QRCODE_print);

/**
 * generate qrcode with specified text
 * return true if success
 * 
 * code.generate('some text')
 **/
STATIC mp_obj_t qrcode_QRCODE_generate(mp_obj_t self_in, mp_obj_t text_in)
{
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

	// ESP_LOGI(TAG, "Encoding below text with ECC LVL %d & QR Code Version %d",
	// 		 self->ecc_level, self->max_version);
	// ESP_LOGI(TAG, "%s", text);

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

/**
 * get length param
 * 
 * code.length()
 **/
STATIC mp_obj_t qrcode_QRCODE_length(mp_obj_t self_in)
{
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	return mp_obj_new_int(self->length);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_length_obj, qrcode_QRCODE_length);

/**
 * get version param
 * 
 * code.version()
 **/
STATIC mp_obj_t qrcode_QRCODE_version(mp_obj_t self_in)
{
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);

	return mp_obj_new_int(self->version);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qrcode_QRCODE_version_obj, qrcode_QRCODE_version);

/**
 * get/set ecc level param
 * 
 * code.ecc_level()
 * code.ecc_level(3)
**/
STATIC mp_obj_t qrcode_QRCODE_ecc_level(size_t n_args, const mp_obj_t *args)
{
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	if (n_args == 1) {
		return mp_obj_new_int(self->ecc_level);
	} else {
		if (mp_obj_is_integer(args[1])) {
			mp_int_t ecc_level = mp_obj_get_int(args[1]);

			if (ecc_level >= QRCODE_ECC_LOW && ecc_level <= QRCODE_ECC_HIGH) {
				self->ecc_level = ecc_level;
			}
		} else {
			mp_raise_ValueError(MP_ERROR_TEXT("invalid arguments"));
		}
	}

	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qrcode_QRCODE_ecc_level_obj, 1, 2, qrcode_QRCODE_ecc_level);
/****************************************/


/***************************
 * QRCODE class definition *
 ***************************/
STATIC const mp_rom_map_elem_t qrcode_QRCODE_locals_dict_table[] = {
	// params
	{MP_ROM_QSTR(MP_QSTR_ecc_level),	MP_ROM_PTR(&qrcode_QRCODE_ecc_level_obj)},
	{MP_ROM_QSTR(MP_QSTR_version),		MP_ROM_PTR(&qrcode_QRCODE_version_obj)},
	{MP_ROM_QSTR(MP_QSTR_length),		MP_ROM_PTR(&qrcode_QRCODE_length_obj)},

	// methods
	{MP_ROM_QSTR(MP_QSTR_generate),		MP_ROM_PTR(&qrcode_QRCODE_generate_obj)},
	{MP_ROM_QSTR(MP_QSTR_print),		MP_ROM_PTR(&qrcode_QRCODE_print_obj)},
	{MP_ROM_QSTR(MP_QSTR_raw_data),		MP_ROM_PTR(&qrcode_QRCODE_raw_data_obj)},
	{MP_ROM_QSTR(MP_QSTR_buffer_data),	MP_ROM_PTR(&qrcode_QRCODE_buffer_data_obj)},
};
STATIC MP_DEFINE_CONST_DICT(qrcode_QRCODE_locals_dict, qrcode_QRCODE_locals_dict_table);

STATIC void qrcode_QRCODE_type_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
	(void) kind;
	qrcode_QRCODE_obj_t *self = MP_OBJ_TO_PTR(self_in);
	mp_printf(print, "<QRCODE version=%d, max_version=%d length=%d, ecc_level=%d, buffer_size=%d>", self->version, self->max_version, self->length, self->ecc_level, self->buffer_size);
}

STATIC MP_DEFINE_CONST_OBJ_TYPE(
	qrcode_QRCODE_type,
	MP_QSTR_QRCODE,
	MP_TYPE_FLAG_NONE,
	print, qrcode_QRCODE_type_print,
	make_new, qrcode_QRCODE_make_new,
	locals_dict, &qrcode_QRCODE_locals_dict
);

mp_obj_t qrcode_QRCODE_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
	enum {
		ARG_ecc_level,
		ARG_max_version,
	};

	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_ecc_level,   MP_ARG_INT, {.u_int = QRCODE_ECC_MED}},
		{MP_QSTR_max_version, MP_ARG_INT, {.u_int = VERSION_MAX}},
	};

	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	// create new object
	qrcode_QRCODE_obj_t *self = m_new_obj(qrcode_QRCODE_obj_t);

	// set parameters
	self->base.type		= &qrcode_QRCODE_type;
	self->ecc_level		= args[ARG_ecc_level].u_int;
	self->max_version	= args[ARG_max_version].u_int;
	self->version		= 0;
	self->length		= 0;
	self->buffer_size	= 0;
	self->buffer		= calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(args[ARG_max_version].u_int));

	if (!self->buffer) {
		mp_raise_msg_varg(
			&mp_type_OSError,
			MP_ERROR_TEXT("out of memory, %ld bytes required."),
			(long) qrcodegen_BUFFER_LEN_FOR_VERSION(args[ARG_max_version].u_int)
		);
	}

	return MP_OBJ_FROM_PTR(self);
}
/****************************************/


/*********************
 * module definition *
 *********************/
STATIC const mp_rom_map_elem_t qrcode_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR___name__),			MP_ROM_QSTR(MP_QSTR_qrcode)},
	{MP_ROM_QSTR(MP_QSTR_QRCODE),			(mp_obj_t) &qrcode_QRCODE_type},

	/* ecc levels*/
	{MP_ROM_QSTR(MP_QSTR_ECC_LOW),			MP_ROM_INT(QRCODE_ECC_LOW)},
	{MP_ROM_QSTR(MP_QSTR_ECC_MED),			MP_ROM_INT(QRCODE_ECC_MED)},
	{MP_ROM_QSTR(MP_QSTR_ECC_QUART),		MP_ROM_INT(QRCODE_ECC_QUART)},
	{MP_ROM_QSTR(MP_QSTR_ECC_HIGH),			MP_ROM_INT(QRCODE_ECC_HIGH)},

	/* versions */
	{MP_ROM_QSTR(MP_QSTR_VERSION_MIN),		MP_ROM_INT(VERSION_MIN)},
	{MP_ROM_QSTR(MP_QSTR_VERSION_MAX),		MP_ROM_INT(VERSION_MAX)},

	/* format*/
	{MP_ROM_QSTR(MP_QSTR_FORMAT_MONO_HLSB),	MP_ROM_INT(FORMAT_MONO_HLSB)},
	{MP_ROM_QSTR(MP_QSTR_FORMAT_RGB565),	MP_ROM_INT(FORMAT_RGB565)},
};
STATIC MP_DEFINE_CONST_DICT(qrcode_module_globals, qrcode_module_globals_table);

const mp_obj_module_t qrcode_user_cmodule = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *) &qrcode_module_globals,
};

#if MODULE_QRCODE_ENABLED
MP_REGISTER_MODULE(MP_QSTR_qrcode, qrcode_user_cmodule);
#endif
