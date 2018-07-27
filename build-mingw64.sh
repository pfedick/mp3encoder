#!/bin/sh
qmake INCLUDEPATH+=/jenkins/local/include -o Makefile mp3encode.pro
mingw32-make -j8 release debug
