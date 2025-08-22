cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=True
cmake --build build/
#gdb --args ctest --test-dir build/tests --verbose --repeat-until-fail 100
ctest --test-dir build/tests --output-on-failure --repeat-until-fail 50
