cmake_minimum_required(VERSION 3.4.1)

# Defines
set(CC_USE_MATH    true)
set(TEXGZ_USE_JPEG true)
set(TEXGZ_USE_PNG  true)
set(VKK_USE_VKUI   true)

# Optional libraries
if(TEXGZ_USE_JPEG)
    set(LIBS_JPEG
        myjpeg)
endif()

# native_app_glue
add_library(native_app_glue
            STATIC

            # Source
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Compiler options
add_compile_options(-Wall -DVK_USE_PLATFORM_ANDROID_KHR -DA3D_GLESv2)

# Main library
add_library(gearsvk
            SHARED

            # Vulkan Wrapper
            ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp

            # Source
            android_main.c
            gears_glsm.c
            gears_overlay.c
            gears_viewAbout.c
            gears_layerHud.c
            gears_renderer.c
            gear.c)

# Submodules
add_subdirectory("jpeg")
add_subdirectory("libcc")
add_subdirectory("libexpat/expat/lib")
add_subdirectory("libpak")
add_subdirectory("libvkk")
add_subdirectory("libxmlstream")
add_subdirectory("texgz")

# Includes
target_include_directories(gearsvk
                           PRIVATE

                           # PATH
                           ${ANDROID_NDK}/sources/third_party/vulkan/src/include/vulkan
                           ${ANDROID_NDK}/sources/third_party/vulkan/src/common
                           ${ANDROID_NDK}/sources/android/native_app_glue)

# Linking
target_link_libraries(gearsvk

                      # Submodule libraries
                      ${LIBS_JPEG}
                      cc
                      xmlstream
                      expat
                      myjpeg
                      pak
                      texgz
                      vkk

                      # NDK libraries
                      android
                      native_app_glue
                      log)
