FONTLIB_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD_C += $(FONTLIB_MOD_DIR)/modfontlib.c

# We can add our module folder to include paths if needed
CFLAGS_USERMOD += -I$(FONTLIB_MOD_DIR)
