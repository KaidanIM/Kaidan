#!/bin/bash

echo "*****************************************"
echo "Building Kaidan binary"
echo "*****************************************"

source /opt/qt*/bin/qt*-env.sh

mkdir ../build; cd ../build/

cmake .. -GNinja -DI18N=1
cmake --build .
