About
=====

Gears VK is a heavily modified port of the famous "gears"
demo.

The Gears demo is an open source project intended to help
developers to learn graphics APIs. The Gears
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

Edit profile as needed for your development environment
before building.

Linux
-----

Command line

	source profile
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

The Gears demo also depends on the following projects.

	https://github.com/jeffboody/libbfs
	https://github.com/jeffboody/libcc
	https://github.com/jeffboody/libexpat
	https://github.com/jeffboody/libsqlite3
	https://github.com/jeffboody/libvkk
	https://github.com/jeffboody/jpeg
	https://github.com/jeffboody/texgz
	https://github.com/lvandeve/lodepng

Screenshot
==========

![Gears VK](docs/gearsvk.jpg?raw=true "Gears VK")

License
=======

This port of Gears VK was implemented by
[Jeff Boody](mailto:jeffboody@gmail.com)
under The MIT License.

	Copyright (c) 2009-2010 Jeff Boody

	Gears for Android is a heavily modified port of the famous "gears" demo.
	As such, it is a derived work subject to the license requirements (below)
	of the original work.

	Copyright (c) 1999-2001  Brian Paul   All Rights Reserved.

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
