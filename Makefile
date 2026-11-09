###############################################################################
# This file is part of MP3Encoder by Patrick Fedick
# Web: https://github.com/pfedick/mp3encoder
###############################################################################
# Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###############################################################################
#
# Zentrale Build-Steuerung. Bildet CMake in eigenen Build-Ordnern pro
# Build-Variante ab, damit release, debug und asan unabhaengig nebeneinander
# existieren.
#
#   make            -> release + debug
#   make release    -> Release-Build   (build/release)
#   make debug      -> Debug-Build     (build/debug)
#   make asan       -> Sanitizer-Build bauen und das Programm damit starten
#   make run        -> Release-Build starten
#   make run-debug  -> Debug-Build starten
#   make clean      -> alle Build-Ordnung entfernen
#
# Optionen:
#   -j N            Anzahlparalleler Compilierprozesse (Standard: 8)
#   GENERATOR=Ninja Generator erzuwingen (Standard: Ninja, falls vorhanden)
#   APP_ARGS="..."  Argumente fuer das zu startende Programm
#   ASAN_SANITIZERS="address,undefined"
#
# Windows (MSYS2/MinGW64): in einer "MSYS2 MinGW x64"-Shell aufrufen, dort
# make und vorzugsweise ninja installieren.
###############################################################################

CMAKE ?= cmake

ifeq ($(OS),Windows_NT)
    EXE = .exe
    ASAN_LEAKS ?= 0    # LeakSanitizer unterstuetzt unter Windows/MSYS kein Detektieren
else
    EXE =
    ASAN_LEAKS ?= 1
endif

# Ninja verwenden, wenn vorhanden; sonst den Standard-Generator von CMake.
GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja)
ifneq ($(GENERATOR),)
    GEN_FLAG := -G "$(GENERATOR)"
else
    GEN_FLAG :=
endif

JOBS ?= 8

BUILD_RELEASE := build/release
BUILD_DEBUG := build/debug
BUILD_ASAN := build/asan

APP_RELEASE := $(BUILD_RELEASE)/mp3encode$(EXE)
APP_DEBUG := $(BUILD_DEBUG)/mp3encode$(EXE)
APP_ASAN := $(BUILD_ASAN)/mp3encode$(EXE)

# UBSan braucht -fno-sanitize-recover, damit Undefiniertes Verhalten gemeldet,
# aber nicht abgebrochen wird; ASan meldet dafuer umso genauer.
ASAN_SANITIZERS ?= address,undefined
ASAN_FLAGS ?= -fsanitize=$(ASAN_SANITIZERS) -fno-omit-frame-pointer -fno-sanitize-recover=all -g -O1
ASAN_OPTIONS ?= detect_leaks=$(ASAN_LEAKS):halt_on_error=0:max_errors=30:allocator_may_return_null=1
UBSAN_OPTIONS ?= print_stacktrace=1:halt_on_error=0
ASAN_LOG := build/asan/sanitizer.log

.PHONY: all release debug asan build_asan run run-debug run-asan configure translations windeploy clean help

all: release debug

configure:
	$(CMAKE) -S . -B $(BUILD_RELEASE) $(GEN_FLAG) -DCMAKE_BUILD_TYPE=Release

release:
	$(CMAKE) -S . -B $(BUILD_RELEASE) $(GEN_FLAG) -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_RELEASE) -j $(JOBS)
	-ln -sf $(BUILD_RELEASE)/compile_commands.json compile_commands.json

debug:
	$(CMAKE) -S . -B $(BUILD_DEBUG) $(GEN_FLAG) -DCMAKE_BUILD_TYPE=Debug
	$(CMAKE) --build $(BUILD_DEBUG) -j $(JOBS)
	-ln -sf $(BUILD_DEBUG)/compile_commands.json compile_commands.json

build_asan:
	$(CMAKE) -S . -B $(BUILD_ASAN) $(GEN_FLAG) -DCMAKE_BUILD_TYPE=Debug -DMP3ENC_SANITIZER=$(ASAN_SANITIZERS)
	$(CMAKE) --build $(BUILD_ASAN) -j $(JOBS)

# Build starten und die Ausgabe des Programms mitgeschnitten. Die
# Sanitizer-Meldungen stehen im Log, am Ende eine Zusammenfassung.
# SHELL=bash, damit PIPESTATUS den Exit-Code des Programms hinter 'tee' liefert.
asan: build_asan
asan: SHELL := /bin/bash
asan:
	@mkdir -p $(dir $(ASAN_LOG)); \
	ASAN_OPTIONS=$(ASAN_OPTIONS) UBSAN_OPTIONS=$(UBSAN_OPTIONS) ./$(APP_ASAN) $(APP_ARGS) 2>&1 | tee $(ASAN_LOG); \
	rc=$${PIPESTATUS[0]}; \
	sed -i 's/\x1b\[[0-9;]*[A-Za-z]//g' $(ASAN_LOG); \
	found=$$(grep -cE 'ERROR: (Address|Leak)Sanitizer|runtime error:' $(ASAN_LOG) || true); \
	echo ""; \
	echo "=== Sanitizer-Report ($(ASAN_SANITIZERS), Log: $(ASAN_LOG)) ==="; \
	echo "Gefundene Meldungen: $$found    Exit-Code des Programms: $$rc"; \
	grep -E 'ERROR: (Address|Leak)Sanitizer|SUMMARY: (Address|Leak)Sanitizer|runtime error:' $(ASAN_LOG) | sort -u | sed 's|^$(CURDIR)/||' | head -40; \
	[ $$rc -eq 0 ] || exit 1

run-asan: asan

run: release
	./$(APP_RELEASE) $(APP_ARGS)

run-debug: debug
	./$(APP_DEBUG) $(APP_ARGS)

# Uebersetzungen aus den .ui-Quellen aktualisieren. .ts und .qm bleiben im
# Repository, das qrc-File greift direkt darauf zu.
LUPDATE ?= $(shell command -v lupdate-qt6 >/dev/null 2>&1 && echo lupdate-qt6 || echo lupdate)
LRELEASE ?= $(shell command -v lrelease-qt6 >/dev/null 2>&1 && echo lrelease-qt6 || echo lrelease)
translations:
	$(LUPDATE) src include -no-obsolete -ts mp3encode_de.ts mp3encode_en.ts
	$(LRELEASE) mp3encode_de.ts mp3encode_en.ts

# Windows: Qt-DLLs und Plugins neben die EXE kopieren.
windeploy: release
	windeployqt --release $(APP_RELEASE)

clean:
	-rm -rf build
	-rm -f compile_commands.json Makefile.Debug Makefile.Release object_script.mp3encode.*
	-rm -f ui_setup.h ui_mp3encode.h ui_editpath.h ui_editaudioprofile.h ui_editid3tag.h

help:
	@echo "Targets: release debug asan build_asan run run-debug run-asan translations windeploy clean"
