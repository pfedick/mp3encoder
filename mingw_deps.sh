#!/bin/sh
ldd release/mp3encode.exe | grep mingw | sort -u | while read ddl_name arrow filename rest
do
	#echo "$filename"
	echo "Source: \"C:/msys64$filename\"; DestDir: \"{app}\"; Flags: ignoreversion;"
	
done

rm -rf release/deploy
windeployqt.exe --dir release/deploy release/mp3encode.exe


