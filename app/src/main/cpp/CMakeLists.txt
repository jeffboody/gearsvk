cmake_minimum_required(VERSION 3.4.1)

# native_app_glue
add_library(native_app_glue
            STATIC

            # Source
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Main library
add_library(gearsvk
            SHARED

            # Vulkan Wrapper
            ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp

            # Source
            android_main.c)

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")
set(${CMAKE_CXX_FLAGS}, "${CMAKE_CXX_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")

target_include_directories(gearsvk
                           PRIVATE

                           # PATH
                           ${ANDROID_NDK}/sources/third_party/vulkan/src/common
                           ${ANDROID_NDK}/sources/android/native_app_glue)

# Linking
target_link_libraries(gearsvk

                      # NDK libraries
                      android
                      native_app_glue
                      EGL
                      GLESv1_CM
                      log)
