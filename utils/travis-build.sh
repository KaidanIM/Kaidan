echo "*****************************************"
echo "Building Kaidan binary"
echo "*****************************************"

mkdir ../build; cd ../build/

cmake .. -DI18N=1
make -j$(nproc)
