# Create an INTERFACE library for our C module.
add_library(usermod_qrcode INTERFACE)

# Add our source files to the lib
target_sources(usermod_qrcode INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modqrcode.c
    ${CMAKE_CURRENT_LIST_DIR}/qrcodegen/qrcodegen.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_qrcode INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/qrcodegen
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_qrcode)
