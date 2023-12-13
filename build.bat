del build
del bin.rel
del doc
del noi_est
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/opt/vcpkg/scripts/buildsystems/vcpkg.cmake 
cmake --build build --config Release 
cmake --install ./build --prefix ./noi_est