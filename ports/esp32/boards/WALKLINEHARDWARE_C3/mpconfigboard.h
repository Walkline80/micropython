// Both of these can be set by mpconfigboard.cmake if a BOARD_VARIANT is
// specified.

#define MICROPY_HW_BOARD_NAME "Walkline Hardware ESP32C3 module"
#define MICROPY_HW_MCU_NAME "ESP32C3"

#define MICROPY_HW_ENABLE_SDCARD        (0)
#define MICROPY_PY_MACHINE_I2S          (0)

#define MODULE_SMARTCONFIG_ENABLED      (1)
#define MODULE_QRCODE_ENABLED           (0)
#define MODULE_FONTLIB_ENABLED          (0)

// Enable UART REPL for modules that have an external USB-UART and don't use native USB.
#define MICROPY_HW_ENABLE_UART_REPL     (1)
