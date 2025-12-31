#!/bin/bash
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
qmake ../VeriQC.pro
make -j2
cd ..

#build VeriQC Console
mkdir build_VeriQC_Console
cd build_VeriQC_Console
qmake ../SubPro/VeriQC_UI.pro DEFINES+=CONSOLE
make -j2
cd ..