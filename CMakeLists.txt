cmake_minimum_required(VERSION 3.16)

# Application Version
set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 2)
set(APP_VERSION_PATCH 0)
set(APP_NAME "Zeal Chess")
set(target_name "chess")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

# -------------------------------
# Project
# -------------------------------
project(${target_name} C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(${target_name}
    src/main.c
    src/chess.c
    src/view.c
    src/splash.c
    src/assets.c
)

# # -------------------------------
# # Include and link SDKs
# # -------------------------------
zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zvb_gfx zvb_sound)

zos_use(coreutils)
find_package(COREUTILS REQUIRED)
target_link_libraries(${target_name} PUBLIC core conio)

zos_use(zgdk)
find_package(ZGDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zgdk)

zos_use(zar)
find_package(ZAR REQUIRED)
target_link_libraries(${target_name} PUBLIC zar)

# -------------------------------
# Assets
# -------------------------------
gif2zeal(${target_name} FILES
    "assets/board.gif"
    BIT 2
    TILEMAP
    UNIQUE
)
gif2zeal(${target_name} FILES
    "assets/pieces.gif"
    BIT 4
)
gif2zeal(${target_name} FILES
    "assets/splash.gif"
    UNIQUE
    TILEMAP
)

# tiled2zeal(${target_name} FILES
#     "assets/splash.tmx"
#     "assets/microbe.tmx"
#     VERBOSE
#     LAYER 0)

copy_assets(${target_name}
    FILES
        "build/assets/board.ztp"
        "build/assets/board.zts"
        "build/assets/board.ztm"
        "build/assets/pieces.ztp"
        "build/assets/pieces.zts"
        "build/assets/splash.ztp"
        "build/assets/splash.zts"
        "build/assets/splash.ztm"

)

# -------------------------------
# Convert to binary
# -------------------------------
zgdk_add_outputs(${target_name})
