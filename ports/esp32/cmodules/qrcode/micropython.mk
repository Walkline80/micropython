QRCODE_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD_C += $(QRCODE_MOD_DIR)/modqrcode.c
SRC_USERMOD_LIB_C += $(QRCODE_MOD_DIR)/qrcodegen/qrcodegen.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(QRCODE_MOD_DIR)
CFLAGS_USERMOD += -I$(QRCODE_MOD_DIR)/qrcodegen
