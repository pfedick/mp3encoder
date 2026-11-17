#/*
# * This file is part of MP3Encoder by Patrick Fedick
# *
# * Copyright (c) 2026 Patrick Fedick
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.
# */

cmake_minimum_required(VERSION 3.22)

# Die Versionsnummer steht in include/version.h und wird dort ausgelesen,
# damit sie auch unter CMake nur an einer Stelle gepflegt werden muss.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h" MP3ENC_VERSION_HEADER)
string(REGEX MATCH "MP3ENCODE_VERSION \"([^\"]*)\"" _ "${MP3ENC_VERSION_HEADER}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "MP3ENCODE_VERSION kann nicht aus include/version.h gelesen werden.")
endif()

project(mp3encode VERSION ${CMAKE_MATCH_1} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# pplib8 wird ausgeliefert ohne Debug-Variante der Bibliotheken. Wenn das
# Projekt im Debug-Build importierte Targets bindest, soll CMake trotzdem
# die Release-Bibliotheken von pplib8 verwenden.
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG "Release;RelWithDebInfo;MinSizeRel;")

# Sanitizer, z.B. -DMP3ENC_SANITIZER=address,undefined
set(MP3ENC_SANITIZER "" CACHE STRING "Sanitizer fuer den Build (z.B. address,undefined)")
set(MP3ENC_EXTRA_CXX_FLAGS "" CACHE STRING "Zusaetzliche Compiler-Schalter")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# pplib8 liegt oft im Homeverzeichnis (make install_home der Bibliothek).
if(DEFINED ENV{HOME} AND EXISTS "$ENV{HOME}/lib64/cmake/pplib8")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}")
elseif(DEFINED ENV{HOME} AND EXISTS "$ENV{HOME}/lib/cmake/pplib8")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}")
endif()

find_package(pplib8 REQUIRED)
if(NOT TARGET pplib::audio)
    message(FATAL_ERROR "pplib8 gefunden, aber das Target pplib::audio fehlt. Installation der Audio-Module pruefen.")
endif()

file(GLOB_RECURSE MP3ENC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE MP3ENC_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE MP3ENC_FORMS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
)

set(MP3ENC_WINDOWS_RES "")
if(WIN32)
    set(MP3ENC_WINDOWS_RES mp3encode.rc)
endif()

# AUTOUIC findet die generierten ui_*.h nicht automatisch in den Unterordnern
# der .ui-Dateien, daher die Suchpfade vor dem Target setzen.
set(CMAKE_AUTOUIC_SEARCH_PATHS "")
foreach(ui_file IN LISTS MP3ENC_FORMS)
    get_filename_component(ui_dir "${ui_file}" DIRECTORY)
    list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${ui_dir}")
endforeach()
list(REMOVE_DUPLICATES CMAKE_AUTOUIC_SEARCH_PATHS)

add_executable(mp3encode
    ${MP3ENC_SOURCES}
    ${MP3ENC_HEADERS}
    ${MP3ENC_FORMS}
    resource.qrc
    ${MP3ENC_WINDOWS_RES}
)

# Unter Windows als GUI-Anwendung ohne Konsole bauen.
if(WIN32)
    set_target_properties(mp3encode PROPERTIES WIN32_EXECUTABLE ON)
endif()

target_include_directories(mp3encode PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# pplib soll die Qt-Interoperabilitaet seiner String-Klassen aktivieren.
target_compile_definitions(mp3encode PRIVATE
    WITH_QT
    PPL_WITH_QT6
    PPL_QT_STRING_UTF8
)

target_link_libraries(mp3encode PRIVATE
    Qt6::Widgets
    pplib::audio
    pplib::core
)

if(MP3ENC_EXTRA_CXX_FLAGS)
    separate_arguments(MP3ENC_EXTRA_CXX_FLAGS UNIX_COMMAND "${MP3ENC_EXTRA_CXX_FLAGS}")
    target_compile_options(mp3encode PRIVATE ${MP3ENC_EXTRA_CXX_FLAGS})
endif()

if(MP3ENC_SANITIZER)
    target_compile_options(mp3encode PRIVATE
        -fsanitize=${MP3ENC_SANITIZER} -fno-omit-frame-pointer -g)
    target_link_options(mp3encode PRIVATE -fsanitize=${MP3ENC_SANITIZER})
endif()

if(WIN32)
    target_link_options(mp3encode PRIVATE -static-libgcc)
endif()

message(STATUS "mp3encode ${PROJECT_VERSION}: Qt ${Qt6_VERSION}, pplib8-Header ${pplib8_VERSION}")
