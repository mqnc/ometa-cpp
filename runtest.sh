build/Release/ometa-cpp test.ometa test.ometa.cpp
mkdir testbuild
cd testbuild
cmake ..
make testometa
./testometa