SMARTCONFIG_MOD_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD_C += $(SMARTCONFIG_MOD_DIR)/modsmartconfig.c

# We can add our module folder to include paths if needed
CFLAGS_USERMOD += -I$(SMARTCONFIG_MOD_DIR)
