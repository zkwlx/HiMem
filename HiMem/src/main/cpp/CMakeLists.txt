# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti")

aux_source_directory(fb_unwinder UNWINDER)
aux_source_directory(fb_unwinder/arm UNWINDER_ARM)
aux_source_directory(fb_unwinder/arm64 UNWINDER_ARM64)
aux_source_directory(fb_unwinder/x86 UNWINDER_X86)
aux_source_directory(fb_unwinder/x86_64 UNWINDER_X86_64)

add_library(
        xhook

        STATIC

        xhook/xh_core.c
        xhook/xh_elf.c
        xhook/xh_jni.c
        xhook/xh_log.c
        xhook/xh_util.c
        xhook/xh_version.c
        xhook/xhook.c
)

add_library(
        clooper

        STATIC

        clooper/looper.cpp
        clooper/looper_test.cpp
)

add_library( # Sets the name of the library.
        himem-native

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        mem_native.cpp
        mem_hook.cpp
        mmap_tracer.cpp
        mem_stack.cpp
#        fb_unwinder/runtime.cpp
#        fb_unwinder/unwinder_wrapper.cpp
#        fb_unwinder/x86_64/unwinder_android_900.cpp
#        fb_unwinder/x86/unwinder_android_900.cpp
#        fb_unwinder/arm64/unwinder_android_900.cpp
#        fb_unwinder/arm/unwinder_android_900.cpp
        ${UNWINDER}
        ${UNWINDER_ARM}
        ${UNWINDER_ARM64}
        ${UNWINDER_X86}
        ${UNWINDER_X86_64}
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        himem-native

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}

        xhook

        clooper)