cmake_minimum_required(VERSION 3.15)
project(debugger VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# === OPTIONS ===
option(BUILD_INTERACTIVE "Build with linenoise (interactive mode)" ON)
option(BUILD_CONFIG "Build with libconfig (config file support)" ON)
option(BUILD_TUI "Build ncurses TUI interface" OFF)  # Désactivé par défaut
option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_WARNINGS "Enable all compiler warnings" ON)

# === COMPILER FLAGS ===
if(ENABLE_WARNINGS)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_compile_options(-g -gdwarf-2)

# === DEPENDENCIES ===
find_package(PkgConfig REQUIRED)

# Libraries for core debugger
find_package(Dwarf REQUIRED)
find_package(Unwind REQUIRED)
find_package(UnwindPtrace REQUIRED)

# linenoise-ng (interactive mode)
if(BUILD_INTERACTIVE)
    pkg_check_modules(LINENOISE REQUIRED linenoise)
    message(STATUS "linenoise found: ${LINENOISE_FOUND}")
endif()

# argtable3 (command line parsing)
pkg_check_modules(ARGTABLE3 REQUIRED argtable3)
message(STATUS "argtable3 found: ${ARGTABLE3_FOUND}")

# libconfig (config file support)
if(BUILD_CONFIG)
    find_package(LibConfig REQUIRED)
    message(STATUS "libconfig found: ${LibConfig_FOUND}")
endif()

# === INCLUDE DIRECTORIES ===
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    /usr/local/include  # Pour stb_ds.h
)

# === SOURCE FILES ===
# Core
set(CORE_SOURCES
    src/core/debugger.c
    src/core/registers.c
    src/core/memory.c
    src/core/breakpoints.c
)

# Display
set(DISPLAY_SOURCES
    src/display/output.c
    src/display/sections.c
    src/display/theme.c
    src/display/formatter.c
)

# CLI
set(CLI_SOURCES
    src/cli/parser.c
    src/cli/repl.c
    src/cli/config.c
)

# Utilities (existing)
set(UTILS_SOURCES
    src/utilities.c
)

# === TARGETS ===

# Debug Console (CLI)
add_executable(debug_console
    src/debug_console.c
    ${CORE_SOURCES}
    ${DISPLAY_SOURCES}
    ${CLI_SOURCES}
    ${UTILS_SOURCES}
)

target_link_libraries(debug_console
    ${Dwarf_LIBRARIES}
    ${Unwind_LIBRARIES}
    ${UnwindPtrace_LIBRARIES}
    ${ARGTABLE3_LIBRARIES}
)

if(BUILD_INTERACTIVE)
    target_link_libraries(debug_console ${LINENOISE_LIBRARIES})
    target_compile_definitions(debug_console PRIVATE HAVE_LINENOISE)
endif()

if(BUILD_CONFIG)
    target_link_libraries(debug_console ${LibConfig_LIBRARIES})
    target_compile_definitions(debug_console PRIVATE HAVE_CONFIG)
endif()

# NCurses TUI (legacy, optionnelle)
if(BUILD_TUI)
    add_executable(interface
        src/interface.c
        ${UTILS_SOURCES}
    )
    # Note: nécessite libncurses, libpanel, etc.
    # target_link_libraries(interface ...)
endif()

# === INSTALLATION ===
install(TARGETS debug_console DESTINATION bin)

# === CONFIGURATION FILES ===
if(BUILD_CONFIG)
    configure_file(
        ${CMAKE_SOURCE_DIR}/config/debuggerrc.default
        ${CMAKE_CURRENT_BINARY_DIR}/debuggerrc.example
        COPYONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/debuggerrc.example
            DESTINATION etc/debugger
            RENAME debuggerrc.example)
endif()
