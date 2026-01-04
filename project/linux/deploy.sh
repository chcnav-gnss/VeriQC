#!/bin/bash

cd ./bin/Release
export PATH=/home/zft/Qt5.12.12/5.12.12/gcc_64/bin:$PATH
cp ../../../../src/VeriQC_UI/Resources/Icons/VeriQC-LOGO.png ./default.png
cp ../../default.desktop ./
linuxdeployqt default.desktop -executable-dir=. -appimage -qmake=/home/zft/Qt5.12.12/5.12.12/gcc_64/bin/qmake
cp /lib/x86_64-linux-gnu/libcrypto.so.1.1 ./lib/
cp /lib/x86_64-linux-gnu/libssl.so.1.1 ./lib/