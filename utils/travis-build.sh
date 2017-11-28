#!/bin/bash

echo "*****************************************"
echo "Building Kaidan binary"
echo "*****************************************"

mkdir ../build; cd ../build/

cmake .. -GNinja -DI18N=1
cmake --build .
