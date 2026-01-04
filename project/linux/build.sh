#!/bin/bash

QMAKE_PATH=/home/zft/Qt5.12.12/5.12.12/gcc_64/bin/qmake

mkdir -p ./bin/Release/lib
# build VeriQC library
rm -rf build_VeriQC_Lib
mkdir build_VeriQC_Lib
cd build_VeriQC_Lib
cmake ..
make
if [ $? -eq 0 ]; then
    echo "Build successful"
else
    echo "Build failed"
    exit 1
fi
cd ..
cp build_VeriQC_Lib/libVeriQC.so ./bin/Release/lib/

# build VeriQC UI
mkdir build_VeriQC
cd build_VeriQC
${QMAKE_PATH} ../VeriQC.pro
make -j2
cd ..

#build VeriQC Console
mkdir build_VeriQC_Console
cd build_VeriQC_Console
${QMAKE_PATH} ../SubPro/VeriQC_UI.pro DEFINES+=CONSOLE
make -j2
cd ..