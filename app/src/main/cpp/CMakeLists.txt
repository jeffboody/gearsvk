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

# ANativeActivity interface
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Compiler options
add_compile_options(-Wall)

# Main library
add_library(gearsvk
            SHARED

            # Source
            gearsvk.c
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
                      log)
