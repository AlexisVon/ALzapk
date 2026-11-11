cmake_minimum_required(VERSION 3.19)
project(ALzapk VERSION 2.0.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Compiler / standard
# ---------------------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Empty build type: flags come from LZA_OPT_LEVEL, not CMAKE_BUILD_TYPE
set(CMAKE_BUILD_TYPE "" CACHE STRING "" FORCE)

set(LZA_OPT_LEVEL "2" CACHE STRING "Optimization level (0/1/2/3)")
set_property(CACHE LZA_OPT_LEVEL PROPERTY STRINGS 0 1 2 3)
if(MSVC)
    add_compile_options(/utf-8)
    # CL has no /O3; levels 2 and 3 both map to /O2
    if(LZA_OPT_LEVEL STREQUAL "0")
        add_compile_options(/Od)
    elseif(LZA_OPT_LEVEL STREQUAL "1")
        add_compile_options(/O1)
    else()
        add_compile_options(/O2)
    endif()
else()
    add_compile_options(-O${LZA_OPT_LEVEL})
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ---------------------------------------------------------------------------
# AlxLib (static, vendored package)
# ---------------------------------------------------------------------------
set(ALXLIB_DIR ${CMAKE_SOURCE_DIR}/lib/alxlib)
set(ALXLIB_INC ${ALXLIB_DIR}/include)
if(WIN32)
    # The Windows package ships DLL import libraries; ship AlxBase.dll/AlxCore.dll beside the executables
    set(ALXLIB_LIBS ${ALXLIB_DIR}/lib/AlxCore.lib ${ALXLIB_DIR}/lib/AlxBase.lib)
else()
    set(ALXLIB_LIBS ${ALXLIB_DIR}/lib/libalxlib.a)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(BUILD_CLI "Build lza-cli" ON)
option(BUILD_UI "Build lza-ui" ON)

# ---------------------------------------------------------------------------
# lza-core (no Qt) — shared by lza-cli and lza-ui, linked statically
# ---------------------------------------------------------------------------
add_library(lza-core STATIC
    src/core/format.cpp
    src/core/progress.cpp
    src/core/pack.cpp
    src/core/archive.cpp
)

target_include_directories(lza-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${ALXLIB_INC}/alxbase
        ${ALXLIB_INC}/alxcore
)

target_link_libraries(lza-core
    PUBLIC
        ${ALXLIB_LIBS}
)

target_compile_definitions(lza-core PRIVATE LZA_VERSION="${PROJECT_VERSION}")

# ---------------------------------------------------------------------------
# lza-cli (no Qt)
# ---------------------------------------------------------------------------
if(BUILD_CLI)
    add_executable(lza-cli
        src/cli/main.cpp
        src/cli/cli.cpp
    )

    target_link_libraries(lza-cli PRIVATE lza-core)
    target_compile_definitions(lza-cli PRIVATE LZA_VERSION="${PROJECT_VERSION}")
endif()

# ---------------------------------------------------------------------------
# lza-ui (Qt6)
# ---------------------------------------------------------------------------
if(BUILD_UI)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    qt_add_executable(lza-ui
        WIN32 MACOSX_BUNDLE
        src/ui/main.cpp
        src/ui/Alexis_Lza.cpp
        src/ui/Alexis_Lza.h
        src/ui/DecodeWidget.cpp
        src/ui/DecodeWidget.h
        src/ui/EncodeWidget.cpp
        src/ui/EncodeWidget.h
        src/ui/FileTableModel.cpp
        src/ui/FileTableModel.h
        src/ui/FSysTree.cpp
        src/ui/FSysTree.h
        src/ui/Global.h
        src/ui/LzaStatusBar.cpp
        src/ui/LzaStatusBar.h
        src/ui/LzaTitleBar.cpp
        src/ui/LzaTitleBar.h
        src/ui/ModeSelector.cpp
        src/ui/ModeSelector.h
        src/ui/TaskControl.cpp
        src/ui/TaskControl.h
        res/ui/lza.qrc
    )

    target_include_directories(lza-ui
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    )

    target_link_libraries(lza-ui PRIVATE
        lza-core
        Qt6::Core
        Qt6::Widgets
    )
endif()

# Windows package: the executables need the AlxLib DLLs beside them
if(WIN32 AND EXISTS "${ALXLIB_DIR}/bin/AlxBase.dll" AND EXISTS "${ALXLIB_DIR}/bin/AlxCore.dll")
    foreach(tgt lza-cli lza-ui)
        if(TARGET ${tgt})
            add_custom_command(TARGET ${tgt} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ALXLIB_DIR}/bin/AlxBase.dll"
                    "${ALXLIB_DIR}/bin/AlxCore.dll"
                    "$<TARGET_FILE_DIR:${tgt}>")
        endif()
    endforeach()
endif()
