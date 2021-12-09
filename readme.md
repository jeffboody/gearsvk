About
=====

Gears VK is a heavily modified port of the famous "gears"
demo to Vulkan/Android/Linux.

The Gears demo is an open source project intended to help
developers learn how to create Vulkan programs. The Gears
demo was originally written by Brian Paul for the OpenGL
API as part of the Mesa3D project. I've ported this app
between multiple languages and graphics APIs over the
years and have found it a very useful learning tool.

This project includes several branches. The main branch
contains a new port to Vulkan via my VKK (Vulkan Kit)
library. The scaler-test branch demonstrates how to use
the VKK library to implement upscaling and super sampling.
The vulkan-tutorial branch contains my original port to
Vulkan which uses the Vulkan library directly and includes
an in depth tutorial. Going forward only the VKK version
will be maintained.

You may find the Github project for Gears VK at

	https://github.com/jeffboody/gearsvk

Send questions or comments to Jeff Boody at jeffboody@gmail.com

Setup
=====

Clone Project
-------------

Clone the gearsvk project https://github.com/jeffboody/gearsvk.

	git clone git@github.com:jeffboody/gearsvk.git
	git checkout -b main origin/main
	git submodule update

Linux
-----

Install library dependencies

	sudo apt-get install libsdl2-2.0 libjpeg-dev

Install Vulkan libraries

	sudo apt-get install libvulkan1 vulkan-utils

Install the LunarG Vulkan SDK

	https://www.lunarg.com/vulkan-sdk/

Android
-------

Download and install Android Studio

	https://developer.android.com/studio

Optionally install Java JDK to build with Gradle from command line

	https://www.oracle.com/technetwork/java/javase/downloads/index.html

Note: if creating a new Android project set the minimum
API level to API 24 (Android 7.0/Nougat) for Vulkan apps.

Building
========

Edit profile.sdl or profile.android as needed for your
development environment before building.

Linux
-----

Command line

	source profile.sdl
	cd app/src/main/cpp
	make
	./gearsvk

Android
-------

Android Studio

	Import Project
	Select the gearsvk folder
	Build > Make Project

References
==========

The following websites were useful in making this port.

	https://www.khronos.org/vulkan/
	https://vulkan.lunarg.com/
	https://wiki.libsdl.org/CategoryVulkan
	https://developer.android.com/ndk/guides/graphics
	https://github.com/vinjn/awesome-vulkan
	https://vulkan-tutorial.com/
	https://developer.nvidia.com/vulkan-shader-resource-binding

In addition to the "Vulkan Programming Guide: The Official
Guide to Learning Vulkan" book.

This app was ported from the Gears2 implementation found here:

	https://github.com/jeffboody/gears2
