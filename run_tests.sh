cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=True
cmake --build build/
ctest --test-dir build/tests

