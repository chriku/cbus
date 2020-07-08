set -e
cmake .. -DCMAKE_CXX_COMPILER=g++-8
make -j8 VERBOSE=1
./cbus_test

