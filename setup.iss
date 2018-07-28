;/*
; * This file is part of MP3Encoder by Patrick Fedick
; *
; * $Author: patrick $
; * $Revision: 1.3 $
; * $Date: 2009/08/24 22:49:39 $
; * $Id: setup.iss,v 1.3 2009/08/24 22:49:39 patrick Exp $
; *
; *
; * Copyright (c) 2009 Patrick Fedick
; *
; * This program is free software: you can redistribute it and/or modify
; * it under the terms of the GNU General Public License as published by
; * the Free Software Foundation, either version 3 of the License, or
; * (at your option) any later version.
; *
; * This program is distributed in the hope that it will be useful,
; * but WITHOUT ANY WARRANTY; without even the implied warranty of
; * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; * GNU General Public License for more details.
; *
; * You should have received a copy of the GNU General Public License
; * along with this program.  If not, see <http://www.gnu.org/licenses/>.
; */


[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{638EF93F-6684-4862-A922-095F672C0AD6}}
AppName=PFP MP3Encoder
AppVerName=PFP MP3Encoder 1.3.0
AppCopyright=Copyright (C) 2018 Patrick Fedick
AppVersion=1.3.0
AppPublisher=Patrick F.-Productions
AppPublisherURL=http://www.pfp.de/
AppSupportURL=http://www.pfp.de/
AppUpdatesURL=http://www.pfp.de/
DefaultDirName={pf}\Patrick F.-Productions\MP3Encoder
DefaultGroupName=Patrick F.-Productions
AllowNoIcons=yes
OutputDir=distfiles
OutputBaseFilename=PFP-MP3Encoder-1.2.0-Setup
SetupIconFile=resourcen\\mp3encoder.ico
Compression=lzma/ultra64
SolidCompression=yes
WizardImageFile=resourcen\setup.bmp
WizardSmallImageFile=resourcen\setup-icon.bmp
LicenseFile=LICENSE.TXT
PrivilegesRequired=none
InfoBeforeFile=README.md
ArchitecturesAllowed=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "release\mp3encode.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.TXT"; DestDir: "{app}"; Flags: ignoreversion
Source: "HISTORY.TXT"; DestDir: "{app}"; Flags: ignoreversion
Source: "resourcen\uninstall.ico"; DestDir: "{app}"; Flags: ignoreversion

Source: "C:/msys64/mingw64/bin/libbz2-1.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libfreetype-6.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libglib-2.0-0.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libgraphite2.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libharfbuzz-0.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libiconv-2.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libicudt61.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libicuin61.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libicuuc61.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libintl-8.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libmp3lame-0.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libmpg123-0.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libpcre-1.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libpcre2-16-0.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libpng16-16.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:/msys64/mingw64/bin/zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libogg-0.dll"; DestDir: "{app}"; Flags: ignoreversion;

; Dependencies of qt imageplugins
Source: "C:\msys64\mingw64\bin\libjpeg-8.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libjasper-4.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libmng-2.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libtiff-5.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libpng16-16.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libwebp-7.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\bin\libwebpdemux-2.dll"; DestDir: "{app}"; Flags: ignoreversion;


 ; Qt specific
Source: "release\deploy\*.dll"; DestDir: "{app}"; Flags: ignoreversion;
; Qt Plugins
Source: "release\deploy\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion;
Source: "release\deploy\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion;
Source: "release\deploy\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion;
Source: "release\deploy\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion;
Source: "release\deploy\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion;
Source: "C:\msys64\mingw64\share\qt5\plugins\audio\qtaudio_windows.dll"; DestDir: "{app}\audio"; Flags: ignoreversion;







[Icons]
Name: "{group}\MP3Encoder"; WorkingDir: "{app}"; Filename: "{app}\mp3encode.exe"
Name: "{group}\{cm:UninstallProgram,MP3Encode}"; IconFilename: "{app}\uninstall.ico"; Filename: "{uninstallexe}"
Name: "{commondesktop}\PFP-MP3Encoder"; Filename: "{app}\mp3encode.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\mp3encode.exe"; Description: "{cm:LaunchProgram,PFP MP3Encoder}"; Flags: nowait postinstall skipifsilent



