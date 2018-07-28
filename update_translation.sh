#!/bin/sh
QT5TRANS=/usr/local/share/qt5/translations
lupdate mp3encode.pro
lrelease mp3encode.pro

for lang in de en
do
	echo $lang
	rm -rf qt_$lang.qm
	lconvert -o qt_$lang.qm $QT5TRANS/qtbase_$lang.qm 
done

 
