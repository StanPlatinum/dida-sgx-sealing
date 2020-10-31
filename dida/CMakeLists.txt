cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
project(DIDA)
ExternalProject_Add(dida
        GIT_REPOSITORY "https://github.com/chathurawidanage/dida"
        GIT_TAG origin/master
        SOURCE_DIR "/home/dajiejie/dida-sgx-sealing/dida/dida"
        BINARY_DIR "/home/dajiejie/dida-sgx-sealing/dida/build"
        INSTALL_DIR "/home/dajiejie/dida-sgx-sealing/dida/install"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=/home/dajiejie/dida-sgx-sealing/dida/install -DWITH_GFLAGS=OFF)
