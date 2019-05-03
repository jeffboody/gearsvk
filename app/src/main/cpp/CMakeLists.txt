cmake_minimum_required(VERSION 3.4.1)

# native_app_glue
add_library(native_app_glue
            STATIC

            # Source
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Compiler options
add_compile_options(-Wall -DVK_USE_PLATFORM_ANDROID_KHR)

# Main library
add_library(gearsvk
            SHARED

            # Vulkan Wrapper
            ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp

            # Source
            android_main.c
            a3d_math.c
            a3d_log.c
            vk_engine.c)

target_include_directories(gearsvk
                           PRIVATE

                           # PATH
                           ${ANDROID_NDK}/sources/third_party/vulkan/src/include/vulkan
                           ${ANDROID_NDK}/sources/third_party/vulkan/src/common
                           ${ANDROID_NDK}/sources/android/native_app_glue)

# Linking
target_link_libraries(gearsvk

                      # NDK libraries
                      android
                      native_app_glue
                      log)
