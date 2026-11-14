cmake_minimum_required(VERSION 3.22.1)
project("nxformat-jni" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Android NDK toolchain - detect if cross-compiling for Android
if(ANDROID)
    # For Android builds, ANDROID_ABI is set by the NDK toolchain
    if(NOT ANDROID_ABI)
        set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI" FORCE)
    endif()
endif()

# Include directories - use local nxformat headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/nxformat
)

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

# Output directory for the built library
if(ANDROID)
    set_target_properties(nxformat-jni PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../jniLibs/${ANDROID_ABI}
    )
else()
    set_target_properties(nxformat-jni PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../jniLibs/arm64-v8a
    )
endif()