# mp3encoder
With this program you can encode WAVE, AIFF, Flac or Ogg files to MP3 or recode existing MP3 files.
Despite its name, mp3encoder also supports encoding to Flac or Aiff.
It can watch folders and automatically encode the files in it or you can drag and drop specific files for encoding.
Existing ID3-Tags or Vorbis Comments (Flac) are kept. A simple ID3-Tag editor is also included.

Mp3encoder comes with a graphical user interface based on Qt6 and PPLib8 and uses mpg123 for decoding and lame for encoding mp3 files. libflac is used for decoding and encoding of Flac files and libvorbis for decoding Ogg Files. Endoding of Ogg Files is not yet supported.

mp3encoder is written by Patrick Fedick <patrick@pfp.de>, https://www.pfp.de/
Source: https://github.com/pfedick/mp3encoder