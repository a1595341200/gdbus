#!/bin/bash

# gdbus-codegen --generate-c-code=Hello framework/interface/Hello.xml
gdbus-codegen --generate-c-code=Hello  --c-namespace hello --interface-prefix com.yft.hello. framework/interface/Hello.xml

cp Hello.h framework/include
cp Hello.c framework/src
rm Hello.*
rm -rf build
cmake -B build

make -C build
