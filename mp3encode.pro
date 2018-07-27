#/*
# * This file is part of MP3Encoder by Patrick Fedick
# * 
# * $Author: patrick $
# * $Revision: 1.15 $
# * $Date: 2010/06/16 18:37:55 $
# * $Id: mp3encode.pro,v 1.15 2010/06/16 18:37:55 patrick Exp $
# *
# * 
# * Copyright (c) 2009 Patrick Fedick
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

TEMPLATE = app
TARGET = mp3encode
QT += core \
    gui \
    widgets

INCLUDEPATH += app 

CONFIG += debug_and_release
CONFIG(debug, debug|release) { 
    win32:TARGET = mp3encode
    unix:TARGET = debug/mp3encode
}
else { 
    win32:TARGET = mp3encode
    unix:TARGET = release/mp3encode
}

HEADERS += include/mp3core.h \
	include/version.h \
    src/forms/setup/setup.h \
    src/forms/mp3encode/mp3encode.h \
    src/forms/editpath/editpath.h \
    src/forms/editaudioprofile/editaudioprofile.h \
    src/forms/editid3tag/editid3tag.h
    
SOURCES += src/core/mp3thread.cpp \
    src/core/mp3core.cpp \
    src/core/main.cpp \
    src/core/config.cpp \
    src/forms/setup/setup.cpp \
    src/forms/mp3encode/mp3encode.cpp \
    src/forms/editpath/editpath.cpp \
    src/forms/editaudioprofile/editaudioprofile.cpp \
    src/forms/editid3tag/editid3tag.cpp
    
FORMS += src/forms/setup/setup.ui \
    src/forms/mp3encode/mp3encode.ui \
    src/forms/editpath/editpath.ui \
    src/forms/editaudioprofile/editaudioprofile.ui \
    src/forms/editid3tag/editid3tag.ui

RESOURCES += resource.qrc
RC_FILE = mp3encode.rc
INCLUDEPATH += include
unix:INCLUDEPATH += /usr/local/include
win32:INCLUDEPATH += C:/mingw/usr/local/include
win32:QMAKE_LIBDIR += C:/mingw/usr/local/lib
win32:QMAKE_LFLAGS += -static-libgcc
CONFIG(debug, debug|release) { 
    # Debug
    unix:LIBS += `ppl7-config --libs debug`
    win32:LIBS += `ppl7-config --libs debug`
    
}
else { 
    # Release
    unix:LIBS += `ppl7-config --libs release`
    win32:LIBS += `ppl7-config --libs release`
    
}

CODECFORSRC = UTF-8
CODECFORTR = UTF-8
