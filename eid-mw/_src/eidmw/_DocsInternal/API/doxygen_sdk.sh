#!/bin/sh


DOXYGEN_CONFIG_FILE="DoxySDK.cfg"

if [[ `uname -s` = "Darwin" ]]; then
../../../ThirdParty/doxygen-1.5.3/macosx/doxygen $DOXYGEN_CONFIG_FILE
else
./doxygen.sh $DOXYGEN_CONFIG_FILE
fi

cp *.css ../../_DocsExternal/C++_Api/html
cp *.jpg ../../_DocsExternal/C++_Api/html


DOXYGEN_CONFIG_FILE="DoxySDK_java.cfg"

if [[ `uname -s` = "Darwin" ]]; then
../../../ThirdParty/doxygen-1.5.3/macosx/doxygen $DOXYGEN_CONFIG_FILE
else
./doxygen.sh $DOXYGEN_CONFIG_FILE
fi

cp *.css ../../_DocsExternal/Java_Api/html
cp *.jpg ../../_DocsExternal/Java_Api/html

