cmake_minimum_required(VERSION 3.22.1)
project("nxformat-jni" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Android NDK toolchain
if(ANDROID)
    set(CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK} CACHE PATH "Android NDK path")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Lichii/externals/nxformat
)

# nxformat C library - we'll build it from source
# For now, create a minimal JNI wrapper that uses the header-only C++ reader
# The actual nxformat C library (libnxformat.so) would need to be built separately
# and placed in jniLibs/arm64-v8a/

# JNI wrapper library
add_library(nxformat-jni SHARED
    nxformat_jni.cpp
    nxformat_wrapper.cpp
)

target_link_libraries(nxformat-jni
    android
    log
    zstd
)

# For the header-only C++ reader, we don't need to link against libnxformat
# The C++ reader (nx_reader.hpp) is self-contained and only needs zstd

# Install the library
set_target_properties(nxformat-jni PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../jniLibs/${ANDROID_ABI}
)