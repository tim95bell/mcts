
#!/bin/bash

cd $(dirname $0)
set -e

pushd ../external

pushd raylib
raylib_build_dir="../../build/raylib"
raylib_debug_install_dir="../../libs/raylib/Debug"
raylib_release_install_dir="../../libs/raylib/Release"

cmake -B$raylib_build_dir -GXcode -DBUILD_SHARED_LIBS=FALSE

cmake --build $raylib_build_dir --config Debug
cmake --install $raylib_build_dir --config Debug --prefix $raylib_debug_install_dir

cmake --build $raylib_build_dir --config Release
cmake --install $raylib_build_dir --config Release --prefix $raylib_release_install_dir

popd

popd
