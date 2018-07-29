#!/bin/sh
###############################################################################
###
### Build artefact script
###
###############################################################################

VERSION=${VERSION:=HEAD}
NAME=mp3encoder

echo "INFO: create artefakt: dist/$NAME-$VERSION.tar.bz2"
rm -rf dist tmp
mkdir -p dist tmp/$NAME-$VERSION
if [ $? -ne 0 ] ; then
    echo "ERROR: could not create directory \"dist\" or \"tmp/$NAME-$VERSION\" "
    exit 1
fi

##################################################################
find src include resourcen HISTORY.TXT LICENSE.TXT *.qm *.ts mp3encode.pro  mp3encode.rc README.md resource.qrc \
    | cpio -pdmv tmp/$NAME-$VERSION > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi
   
##################################################################
# TAR-File bauen
cd tmp
tar -cjf ../dist/$NAME-$VERSION.tar.bz2 $NAME-$VERSION
if [ $? -ne 0 ] ; then
    echo "ERROR: failed to create tar-file"
    exit 1
fi
echo "INFO: artefakt done"
exit 0