#!/bin/bash

# gdbus-codegen --generate-c-code=Test framework/interface/Test.xml
gdbus-codegen --generate-c-code=Test --c-namespace gdbus --interface-prefix com.yao.xie.gdbus. framework/interface/Test.xml

cp Test.h framework/include
cp Test.c framework/src
rm Test.*
# rm -rf build
cmake -B build

make -C build
