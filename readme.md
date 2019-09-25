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

Since the Vulkan API is significantly different than the
OpenGL API for which the code was based I've also included
a short Vulkan Tutorial below. This section
explains some of the design choices that I've made while
porting the app. Additionally you may find several
Graphviz documents under the docs directory that
can be used as companion documentation to the "Vulkan
Programming Guide: The Official Guide to Learning Vulkan".
These dot files may be viewed with the xdot program.

This project includes two branches. The master branch
contains my original port to Vulkan and uses the Vulkan
library directly. The master-vkk branch contains a new
port to Vulkan via my VKK (Vulkan Kit) wrapper library.
Going forward only the VKK version will be maintained.

You may find the Github project for Gears VK at

	https://github.com/jeffboody/gearsvk

Send questions or comments to Jeff Boody at jeffboody@gmail.com

Setup
=====

Clone Project
-------------

Clone the gearsvk project https://github.com/jeffboody/gearsvk.

	git clone git@github.com:jeffboody/gearsvk.git
	git checkout -b master-vkk origin/master-vkk
	git submodule update

Linux
-----

Install library dependencies

	sudo apt-get install libsdl2-2.0 libjpeg-dev libpng-dev

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

Command line (requires Java for Gradle)

	source profile.android
	./build.sh
	./install.sh

Android Studio

	Import Project
	Select the gearsvk folder
	Build > Make Project

Vulkan Tutorial
===============

Introduction
------------

Vulkan is a low-level graphics API which is the spiritual
successor to the OpenGL and OpenGL ES APIs.
The low-level nature of Vulkan causes apps which
utilize the API to implement functionality which would have
been previously implemented by the OpenGL graphics driver.
For example, consider an OpenGL app which updates a uniform
every frame via glUniform1f. The OpenGL app only tracks a
single object handle while the OpenGL graphics driver
handles the allocation of the underlying memory. When the
OpenGL app updates the uniform per frame the OpenGL
graphics driver must actually allocate multiple copes of
the underlying memory because one copy is needed for each
framebuffer in the swapchain (e.g. triple buffered
surfaces are common). A Vulkan app must be aware of when
a uniform buffer is still in use by the graphics hardware
to ensure that it doesn't corrupt the memory.
Additionally the Vulkan app is responsible for actually
allocating the device memory and choosing the memory type
based on the usage requirements. The Gears VK app handles
dynamic buffers that are updated per-frame by creating
arrays of buffers/descriptors/etc that are the same
size as the swapchain image count.

Vulkan functions typically take structures as parameters
rather than passing each parameter individually. Only one
argument must be copied in the function call which results
in increased CPU performance. This pattern can significantly
reduce function call overhead for complicated graphics apps.
I recommend using the structure "dot" notation for
initializing these structures and to improve code quality.
The definition of structures can be determined easily by
searching the web for "Khronos vkFunction/VkStructure"
then copying the structure definition. e.g.

	VkApplicationInfo a_info =
	{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext              = NULL,
		.pApplicationName   = "My App",
		.applicationVersion = VK_MAKE_VERSION(1,0,0),
		.pEngineName        = "My Engine",
		.engineVersion      = VK_MAKE_VERSION(1,0,0),
		.apiVersion         = VK_MAKE_VERSION(1,0,0),
	};

The Vulkan API also reduces CPU overhead by performing a
minimal amount of runtime error checking. It is also very
easy to use the Vulkan API incorrectly in ways that would
have generated errors in the OpenGL programming language.
The lack of runtime error checking can even lead to crashes
when the app uses the Vulkan API incorrectly. To address
this limitation the Vulkan creators have designed a set of
Validation Layers that can be enabled for debugging and
validation. See the Instance section for more details. It is
important to test the app with validation layers enabled
because there can be problems even when there are no crashes
or visible rendering issues.

Shaders and the programmable graphics stages represent one
of the most important aspects of the Vulkan API. For
developers who are porting apps from OpenGL you can breathe
a sigh of relief that your existing OpenGL shaders can be
used with minimal changes. In particular, the binding
mechanism for the input and output variables is different
than that of OpenGL. In the OpenGL API, the app would
explicitly query for the attribute and uniform locations
(e.g.  glGetAttribLocation and glGetUniformLocation).
In the Vulkan API, the shaders defines locations, sets and
binding numbers that must also be known to the app source
code. See the Shaders section for more details.

The other key differences with respect to shaders are the
offline shader compiler and SPIR-V intermediate binary
representation. The offline shader compiler has many
advantages over runtime shader compilation including
improved runtime performance, reduced application
size, reduced driver complexity, shared compiler
between driver vendors. The SPIR-V intermediate binary
representation can also be targeted by multiple programming
languages making it possible for developers to create new
shading languages or enable existing programming languages
to run on the GPU. It is important to note that the SPIR-V
intermediate binary representation is not machine code
and a final compilation step is required by the Vulkan
driver to convert to the GPU vendors proprietary
GPU instruction set or perform vendor specific
optimizations.

The following sections will walk you through the platform
setup, creation of Vulkan graphics state and drawing
commands used by the Gears VK port.

Main
----

The main function for Linux is rather trivial and mostly
consists of a loop that polls for SDL events and initiating
drawing for the gears renderer. See the gearsvk.c file for
more details.

On Android the main function is implemented by creating an
Android NativeActivity. You may find it useful to review the
first few commits in the gearsvk project to see how to
create a Native Activity. The Android native app glue
provides a simplified interface for pure native apps which
just require a minimal subset of the Android API. As such,
the android_main() function is essentially equivalent to the
normal C main() entry point.

Of particular interest to Vulkan apps is that the Android
native app glue provides an event called
APP_CMD_WINDOW_RESIZED however this does not appear
to be hooked up properly in the Android framework. So, in
order to detect window resizes you should compare the
ANativeWindow size with the previous frame before calling
your draw function.

	int32_t width;
	int32_t height;
	width  = ANativeWindow_getWidth(app->window);
	height = ANativeWindow_getHeight(app->window);

The following discussion regarding the NativeActivity can be
ignored if you are only interested in the Vulkan API.

I've choosen to extend the NativeActivity Java class in
order to provide additional functionality that isn't
available to pure native apps. At present, the only
functionality demonstrated is an EXIT command that lets the
user press the back button to quit the app. I'll briefly
summarize how this works in case you require additional
functionality provided by the Android Java API. A key
feature of the native app glue is that it maintains a queue
of events which are submitted to the app on its event thread
then spawns the rendering thread for the android_main()
function. The app may call the cmd_fn callback which uses
JNI commands to send a command/message from the rendering
thread to the Java CallbackCmd function in the rendering
thread. The Java CallbackCmd function sends an event from
the rendering thread to the event thread. This event will be
received by the handleMessage interface on the event thread.
This complicated sequence of passing events is required
since some Java functions may only be called from the Java
event thread.

See the android_main.c and GearsVK.java files for more
details.

SDL2
----

Gears VK will use SDL 2.0 to create a window for the Linux
platform. Initialization of the SDL window requires several
changes compared to an OpenGL window. First, you will need
to use the SDL_Vulkan functions instead of the SDL_GL
functions. Secondly, the flag parameter to the
SDL_CreateWindow() function should include SDL_WINDOW_VULKAN
instead of SDL_WINDOW_OPENGL. SDL also requires that the app
requests the VK_KHR_xlib_surface extension when creating the
Vulkan instance. See the Instance section for more detais.
And finally, the app should use the SDL_Vulkan_CreateSurface
function to create the Vulkan surface. See the Surface
section for more details.

Instance
--------

The creation of a VkInstance object is required for a
Vulkan app to initialize and interface with the Vulkan
library.

In order to create surfaces that may be displayed on the
screen, the Vulkan app must enable the following platform
specific extensions when creating the VkInstance.

	uint32_t    extension_count   = 2;
	const char* extension_names[] =
	{
		"VK_KHR_surface",
		#ifdef ANDROID
			"VK_KHR_android_surface"
		#else
			"VK_KHR_xlib_surface"
		#endif
	};

The VkInstance object also requires for the app to provide
some information about itself. This information may be
useful for several purposes including debugging, performance
tuning or enabling driver workarounds.

	VkApplicationInfo a_info =
	{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext              = NULL,
		.pApplicationName   = "App Name",
		.applicationVersion = VK_MAKE_VERSION(1,0,0),
		.pEngineName        = "Engine Name",
		.engineVersion      = VK_MAKE_VERSION(1,0,0),
		.apiVersion         = VK_MAKE_VERSION(1,0,0),
	};

The Vulkan API supports a Layer Framework that allows for
API calls to be intercepted for various purposes. The
LunarG Vulkan SDK includes several layers for validating
and debugging Vulkan apps. In particular, I recommend that
you test the app with the standard validation meta layer
which is a combination of several useful layers. These
layers may have an adverse impact on performance so you
should disable them in the release version of the app.

	uint32_t    layer_count   = 1;
	const char* layer_names[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	VkInstanceCreateInfo ic_info =
	{
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext                   = NULL,
		.flags                   = 0,
		.pApplicationInfo        = &a_info,
		.enabledLayerCount       = layer_count,
		.ppEnabledLayerNames     = layer_names,
		.enabledExtensionCount   = extension_count,
		.ppEnabledExtensionNames = extension_names
	};

	VkInstance instance;
	if(vkCreateInstance(&ic_info, NULL,
	                    &instance) != VK_SUCCESS)

Surface
-------

The Vulkan app must create a VkSurface to render to the
display. The surface is a platform specific handle to the
operating system and must be created accordingly.

	VkSurface surface;
	#ifdef ANDROID
		VkAndroidSurfaceCreateInfoKHR as_info =
		{
			.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.pNext  = NULL,
			.flags  = 0,
			.window = app->window
		};

		if(vkCreateAndroidSurfaceKHR(instance,
		                             &as_info, NULL,
		                             &surface) != VK_SUCCESS)
	#else
		if(SDL_Vulkan_CreateSurface(window, instance,
		                            &surface) == SDL_FALSE)
	#endif

Physical Device
---------------

The Vulkan app must select a physical device (GPU) for
submitting rendering commands. Typically only a single
physical device will exist however it is also possible for
multi-GPU configurations to exist.

The vkEnumeratePhysicalDevices function follows a pattern
used by many Vulkan functions which return arrays of
objects. The function is typically called twice to query
the physical device count and a second time to receive the
array of physical devices. The first call passes NULL to
the physical device array to indicate it is querying the
physical device count.

	// query the physical device count
	uint32_t physical_device_count;
	if(vkEnumeratePhysicalDevices(instance,
	                              &physical_device_count,
	                              NULL) != VK_SUCCESS)

	// select the first physical device
	VkResult result;
	VkPhysicalDevice physical_device;
	physical_device_count = 1;
	result = vkEnumeratePhysicalDevices(instance,
	                                    &physical_device_count,
	                                    &physical_device);
	if((result == VK_SUCCESS) || (result == VK_INCOMPLETE))

In most cases it should be fine to select the first
physical device but you may need to perform additional
queries to ensure the selected device supports the required
features. The following functions are useful for this
purpose.

	vkGetPhysicalDeviceProperties
	vkGetPhysicalDeviceFeatures
	vkGetPhysicalDeviceMemoryProperties
	vkGetPhysicalDeviceQueueFamilyPropertiesLogical
	vkEnumerateDeviceLayerProperties
	vkEnumerateDeviceExtensionProperties

Logical Device
--------------

The logical device is the Vulkan API construct for a
particular physical device that represents a particular
configuration of the physical device and queue(s) for
submitting commands. The app must select a compatible
device queue from the physical device which may support
multiple queues of different types. The following code
selects a graphics device queue that also supports the
platform specific surface.

	uint32_t qfp_count;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
	                                         &qfp_count,
	                                         NULL);

	VkQueueFamilyProperties* qfp;
	qfp = (VkQueueFamilyProperties*)
	      calloc(qfp_count,
	             sizeof(VkQueueFamilyProperties));

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
	                                         &qfp_count,
	                                         qfp);

	int i;
	int has_index = 0;
	queue_family_index = 0;
	for(i = 0; i < qfp_count; ++i)
	{
		if(qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VkBool32 supported;
			if(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
			                                        i, surface,
			                                        &supported) != VK_SUCCESS)

			if(supported && (has_index == 0))
			{
				// select the first supported device queue
				queue_family_index = i;
				has_index = 1;
			}
		}
	}

	float queue_priority = 0.0f;
	VkDeviceQueueCreateInfo dqc_info =
	{
		.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext            = NULL,
		.flags            = 0,
		.queueFamilyIndex = queue_family_index,
		.queueCount       = 1,
		.pQueuePriorities = &queue_priority
	};

The app must request the swapchain extension in order to
present images to the display when creating the logical
device.

	uint32_t    extension_count   = 1;
	const char* extension_names[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo dc_info =
	{
		.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext                   = NULL,
		.flags                   = 0,
		.queueCreateInfoCount    = 1,
		.pQueueCreateInfos       = &dqc_info,
		.enabledLayerCount       = 0,
		.ppEnabledLayerNames     = NULL,
		.enabledExtensionCount   = extension_count,
		.ppEnabledExtensionNames = extension_names,
		.pEnabledFeatures        = NULL
	};

	if(vkCreateDevice(physical_device, &dc_info,
	                  NULL, &device) != VK_SUCCESS)

Finally, we store a reference to the device queue for
submitting commands to the GPU.

	vkGetDeviceQueue(device,
	                 queue_family_index,
                     0, &queue);

Free the temporary queue family properties.

	free(qfp);

Cache and Pools
---------------

This section introduces pipeline caches, command pools and
descriptor pools. These constructs provide low level access
to optimize the creation of their corresponding objects.
There are not analogous constructs in the OpenGL API
because these would have been implemented transparently
to the app by the OpenGL graphics driver (if at all). These
constructs are required but can be treated as boilerplate
code by most apps.

The pipeline cache is used to optimize the creation of
graphics and compute pipelines. Operations involving the
pipeline cache is thread safe and the same pipeline cache
object may be used in multiple threads simultaneously. See
the Graphics Pipeline section doe more details.

	VkPipelineCacheCreateInfo pc_info =
	{
		.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext           = NULL,
		.flags           = 0,
		.initialDataSize = 0,
		.pInitialData    = NULL
	};

	VkPipelineCache pipeline_cache;
	if(vkCreatePipelineCache(device, &pc_info, NULL,
	                         &pipeline_cache) != VK_SUCCESS)

The command pool is used to optimize the allocation of
command buffers. Operations involving command pools are NOT
thread safe. See the Command Buffers and Logical Device
sections for more details.

	VkCommandPoolCreateInfo cpc_info =
	{
		.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext            = NULL,
		.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family_index
	};

	VkCommandPool command_pool;
	if(vkCreateCommandPool(device, &cpc_info, NULL,
	                       &command_pool) != VK_SUCCESS)

The descriptor pool is used to optimize the allocation of
descriptor sets. The descriptor pools are a bit complicated
and I don't yet understand the best practices for using
them. Be aware that you need to specify a descriptor pool
size for each type of descriptor that is required.
Fragmentation of the descriptor pool is possible and I
recommend that you read the Khronos documentation pages for
VkDescriptorPoolCreateInfo and
VkDescriptorPoolCreateFlagBits to understand this in more
detail. In particular, the descriptor pool supports the
VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag
which allows the app to free descriptors. Without this flag
the app should NOT free individual descriptors allocated
from the pool using vkFreeDescriptorSets. Operations
involving descriptor pools are NOT thread safe. See the
Command Buffers and Logical Device sections for more
details.


	VkDescriptorPoolSize pool_size =
	{
		.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 64
	};

	VkDescriptorPoolCreateInfo dp_info =
	{
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext         = NULL,
		.flags         = 0,
		.maxSets       = 64,
		.poolSizeCount = 1,
		.pPoolSizes    = &pool_size
	};

	VkDescriptorPool descriptor_pool;
	if(vkCreateDescriptorPool(device, &dp_info, NULL,
	                          &descriptor_pool) != VK_SUCCESS)

Swapchain
---------

The swapchain is a collection of color buffer images
corresponding to a native platform surface that can be
presented to a display.

To configure the swapchain we must query the physical device
surface capabilities.

	VkSurfaceCapabilitiesKHR caps;
	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
	                                             surface,
	                                             &caps) != VK_SUCCESS)

Default to triple buffered rendering when supported.

	uint32_t minImageCount = 3;
	if((caps.maxImageCount > 0) &&
	   (minImageCount > caps.maxImageCount))
	{
		minImageCount = caps.maxImageCount;
	}
	else if(minImageCount < caps.minImageCount)
	{
		minImageCount = caps.minImageCount;
	}

Query the number of physical device surface formats
supported.

	uint32_t sf_count;
	if(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
	                                        surface,
	                                        &sf_count,
	                                        NULL) != VK_SUCCESS)

Fill the surface format array with physical device surface
formats supported.

	VkSurfaceFormatKHR* sf;
	sf = (VkSurfaceFormatKHR*)
	     calloc(sf_count, sizeof(VkSurfaceFormatKHR));

	if(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
	                                        surface,
	                                        &sf_count,
	                                        sf) != VK_SUCCESS)

Select an RGBA-8888 color format if possible.

	swapchain_format      = sf[0].format;
	swapchain_color_space = sf[0].colorSpace;
	int i;
	for(i = 0; i < sf_count; ++i)
	{
		if((sf[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) &&
		   ((sf[i].format == VK_FORMAT_R8G8B8A8_UNORM) ||
		    (sf[i].format == VK_FORMAT_B8G8R8A8_UNORM)))
		{
			swapchain_format      = sf[i].format;
			swapchain_color_space = sf[i].colorSpace;
			break;
		}
	}

Query the number of physical device surface present modes
supported.

	uint32_t pm_count;
	if(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
	                                             surface,
	                                             &pm_count,
	                                             NULL) != VK_SUCCESS)
	{
		goto fail_present_modes1;
	}

Fill the physical device surface present modes array.

	VkPresentModeKHR* pm;
	pm = (VkPresentModeKHR*)
	     calloc(pm_count, sizeof(VkPresentModeKHR));
	if(pm == NULL)

	if(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
	                                             surface,
	                                             &pm_count,
	                                             pm) != VK_SUCCESS)

Select the VK_PRESENT_MODE_FIFO_KHR if possible to present
rendered buffers in the order rendered without skipping
any.

	VkPresentModeKHR present_mode = pm[0];
	for(i = 0; i < pm_count; ++i)
	{
		if(pm[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			present_mode = pm[i];
			break;
		}
	}

Select the VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
preTransform mode if possible.

	VkSurfaceTransformFlagBitsKHR preTransform;
	if(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = caps.currentTransform;
	}

Create the swapchain using the parameters determined above
and be sure to save the swapchain_extent for use by other
sections.

	VkSwapchainCreateInfoKHR sc_info =
	{
		.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext                 = NULL,
		.flags                 = 0,
		.surface               = surface,
		.minImageCount         = minImageCount,
		.imageFormat           = swapchain_format,
		.imageColorSpace       = swapchain_color_space,
		.imageExtent           = caps.currentExtent,
		.imageArrayLayers      = 1,
		.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &queue_family_index,
		.preTransform          = preTransform,
		.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode           = present_mode,
		.clipped               = VK_TRUE,
		.oldSwapchain          = VK_NULL_HANDLE
	};

	swapchain_extent = caps.currentExtent;

	if(vkCreateSwapchainKHR(device, &sc_info, NULL,
	                        &swapchain) != VK_SUCCESS)

Query the swapchain image count and validate the swapchain
image count across resizes to ensure that our Vulkan object
arrays are still compatible. See the Surface Resizes
section for more details.

	uint32_t count = 0;
	if(vkGetSwapchainImagesKHR(device, swapchain, &count,
	                           NULL) != VK_SUCCESS)

	if(swapchain_image_count &&
	   (swapchain_image_count != count))
	swapchain_image_count = count;

Fill the swapchain images array to be used for the
framebuffer color buffers. See the Framebuffer section for
more details.

	swapchain_images = (VkImage*)
	                   calloc(swapchain_image_count,
	                          sizeof(VkImage));

	uint32_t image_count = swapchain_image_count;
	if(vkGetSwapchainImagesKHR(device, swapchain,
	                           &image_count,
	                           swapchain_images) != VK_SUCCESS)

Create a swapchain fence for each swapchain image. The
swapchain fence is used to ensure that all commands which
were previously queued via vkQueueSubmit() have completed
before attempting to record more commands. See the Drawing
Commands section for more details.

	swapchain_fences = (VkFence*)
	                   calloc(swapchain_image_count,
	                          sizeof(VkFence));

	for(i = 0; i < image_count; ++i)
	{
		VkFenceCreateInfo f_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		if(vkCreateFence(device, &f_info, NULL,
		                 &swapchain_fences[i]) != VK_SUCCESS)
	}

And finally, since the swapchain depends on the surface and
must handle resizes as described in the Surface Resizes
section.

Renderpass
----------

The Vulkan API renderpass construct does not have an
analogous construct to the OpenGL API. However, OpenGL apps
perform functionally equivalent rendering more informally
through multipass rendering. The Vulkan API formalizes this
by requiring an app to describe the attachments (e.g. color
and depth buffers) used by the renderpass across one or more
subpasses. The precise description of a renderpass may lead
to improved performance because the GPU may be able to
access buffers more efficiently. Several of the attachment
description fields deserve further discussion.

The loadOp can be set to CLEAR which is roughly equivalent
to calling glClear() where the values come from the
parameters to vkCmdBeginRenderPass(). When the loadOp is set
to LOAD the previous contents of the buffer are preserved.
When the loadOp is set to DONT_CARE the buffer contents are
undefined and the app is expected to render to the entire
area.

The storeOp can be set to STORE which causes the buffer
values to be saved by the renderpass as is the typical
behavior of OpenGL. The storeOp can also be set to
DONT_CARE which allows the Vulkan driver to discard the
values at the end of the renderpass. The discard behavior
is equivalent to the EXT_discard_framebuffer in OpenGL.

The image layouts are essentially hints which allow the
Vulkan driver to access the image memory in an optimal
manner. As rendering occurs you may need to transition the
image layer to match the desired usage. For the first
attachment (color buffer) the initialLayout will be
UNDEFINED for the first frame and PRESENT_SRC_KHR for the
remaining frames based on my experiments with the Vulkan
validation layers. See the Drawing Commands section to see
how to transition the layout to COLOR_ATTACHMENT_OPTIMAL for
rendering. The renderpass automatically transitions the
image layout to the finalLayout. In this case, we want
PRESENT_SRC_KHR so that the image may be displayed on the
screen. The depth buffer will only be read/written by the
GPU so we can use the DEPTH_STENCIL_ATTACHMENT_OPTIMAL
image layout. However, my experience with the Vulkan
validation layers suggests that the initial layout is
UNDEFINED for the first time a depth image is used so
each one in the swapchain should be transitioned before
first use. Failure to transition an image layout to the
correct usage may prevent the Vulkan hardware from reading
or writing to a buffer since some image layouts are only
supported for specific uses.

	VkAttachmentDescription attachments[2] =
	{
		{
			.flags          = 0,
			.format         = swapchain_format,
			.samples        = VK_SAMPLE_COUNT_1_BIT,
			.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
		{
			.flags          = 0,
			.format         = VK_FORMAT_D16_UNORM,
			.samples        = VK_SAMPLE_COUNT_1_BIT,
			.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};

The attachment reference describes the attachment
description index and the layout to be used for the
attachment during the subpass. Note that each subpass may
use a subset of attachments and attachments may be used in
multiple subpasses.

	VkAttachmentReference color_attachment =
	{
		.attachment = 0,
		.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depth_attachment =
	{
		.attachment = 1,
		.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass =
	{
		.flags = 0,
		.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount    = 0,
		.pInputAttachments       = NULL,
		.colorAttachmentCount    = 1,
		.pColorAttachments       = &color_attachment,
		.pResolveAttachments     = NULL,
		.pDepthStencilAttachment = &depth_attachment,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments    = NULL,
	};

	VkRenderPassCreateInfo rp_info =
	{
		.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext           = NULL,
		.flags           = 0,
		.attachmentCount = 2,
		.pAttachments    = attachments,
		.subpassCount    = 1,
		.pSubpasses      = &subpass,
		.dependencyCount = 0,
		.pDependencies   = NULL
	};

	if(vkCreateRenderPass(device, &rp_info, NULL,
	                      &render_pass) != VK_SUCCESS)

Depth Buffer
------------

The Gears VK app uses a simple 16-bit depth buffer. In the
Vulkan API this consists of VkImage, VkDeviceMemory and
VkImageView constructs.

The VkImage describes the depth buffers format, size and
usage requirements. The swapchain extent is described in the
Swapchain section. The queue family is ignored since the
sharingMode is VK_SHARING_MODE_EXCLUSIVE. The image layouts
are a particullarly confusing concept that is discussed in
the Renderpass and Drawing sections. And finally, since the
depth buffer depends on the surface size, it must be resized
when the surface is resized as described in the Surface
Resizes section.

	VkImageCreateInfo i_info =
	{
		.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext     = NULL,
		.flags     = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format    = VK_FORMAT_D16_UNORM,
		.extent    =
		{
			swapchain_extent.width,
			swapchain_extent.height,
			1
		},
		.mipLevels   = 1,
		.arrayLayers = 1,
		.samples     = VK_SAMPLE_COUNT_1_BIT,
		.tiling      = VK_IMAGE_TILING_OPTIMAL,
		.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &queue_family_index,
		.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkImage depth_image;
	if(vkCreateImage(device, &i_info, NULL,
	                 &depth_image) != VK_SUCCESS)

The app must allocate device memory for the depth image. To
do so, we must determine the memory requirements of the
image and combine those with our own requirements. Since the
depth buffer will only be accessed by the GPU we do not need
to specify custom flags to determine the memory type index.
See the getMemoryTypeIndex section for a utility which
selects a memory type.

	VkMemoryRequirements mr;
	vkGetImageMemoryRequirements(device, depth_image, &mr);

	VkFlags  mp_flags = 0;
	uint32_t mt_index;
	if(getMemoryTypeIndex(mr.memoryTypeBits,
	                      mp_flags, &mt_index) == 0)

	VkMemoryAllocateInfo ma_info =
	{
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext           = NULL,
		.allocationSize  = mr.size,
		.memoryTypeIndex = mt_index
	};

	VkDeviceMemory depth_memory;
	if(vkAllocateMemory(device, &ma_info, NULL,
	                    &depth_memory) != VK_SUCCESS)

The depth image must be bound to the depth memory.

	if(vkBindImageMemory(device, depth_image, depth_memory,
	                     0) != VK_SUCCESS)

The depth buffer will be used as an VkImageView attachment
to the framebuffer. The image view may seem a bit redundant
for the simple use case of depth buffers however more
advanced uses exist which may require the ability to swap
color channels, render to a specific mip level or a
particular array index. See the Framebuffer section for
more details.

	VkImageViewCreateInfo iv_info =
	{
		.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext      = NULL,
		.flags      = 0,
		.image      = depth_image,
		.viewType   = VK_IMAGE_VIEW_TYPE_2D,
		.format     = VK_FORMAT_D16_UNORM,
		.components =
		{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange =
		{
			.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1
		}
	};

	VkImageView depth_image_view;
	if(vkCreateImageView(device, &iv_info, NULL,
	                     &depth_image_view) != VK_SUCCESS)

Framebuffer
-----------

The Vulkan API requires a framebuffer for each swapchain
image. Each framebuffer consists of a color buffer
and depth buffer (when enabled), each wrapped by an image
view. The GPU will only process one frame at a time and the
depth buffer is discarded every frame. Therefore the depth
buffer can be reused rather than requiring unique depth
buffers. The framebuffer will be compatible with the
specific render_pass and other renderpasses compatible with
the render_pass. See the Khronos documentation page for
VkFramebufferCreateInfo to understand this in more detail.
See the Swapchain section for details on retrieving
the swapchain images (color buffers) from the swapchain.
See the Depth section for details on creating the depth
image view. And finally, since the framebuffer depends on
the surface size, it must be resized when the surface is
resized as described in the Surface Resizes section.

	int i;
	for(i = 0; i < swapchain_image_count; ++i)
	{
		VkImageViewCreateInfo iv_info =
		{
			.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext      = NULL,
			.flags      = 0,
			.image      = swapchain_images[i],
			.viewType   = VK_IMAGE_VIEW_TYPE_2D,
			.format     = swapchain_format,
			.components =
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange =
			{
				.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1
			}
		};

		if(vkCreateImageView(device, &iv_info, NULL,
		                     &framebuffer_image_views[i]) != VK_SUCCESS)

		VkImageView attachments[2] =
		{
			framebuffer_image_views[i],
			depth_image_view
		};

		VkFramebufferCreateInfo f_info =
		{
			.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext           = NULL,
			.flags           = 0,
			.renderPass      = render_pass,
			.attachmentCount = 2,
			.pAttachments    = attachments,
			.width           = swapchain_extent.width,
			.height          = swapchain_extent.height,
			.layers          = 1,
		};

		if(vkCreateFramebuffer(device, &f_info, NULL,
		                       &framebuffers[i]) != VK_SUCCESS)
	}

Command Buffers
---------------

Command buffers are used to record commands that can
subsequently be submitted to a device queue. One command
buffer is required for each swapchain image because the CPU
will be producing the command buffers while the GPU will be
consuming the command buffers asynchronously.

	VkCommandBuffer* command_buffers;
	command_buffers = (VkCommandBuffer*)
	                  calloc(swapchain_image_count,
	                         sizeof(VkCommandBuffer));

	VkCommandBufferAllocateInfo cba_info =
	{
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext              = NULL,
		.commandPool        = command_pool,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = swapchain_image_count
	};

	if(vkAllocateCommandBuffers(device, &cba_info,
	                            command_buffers) != VK_SUCCESS)

Semaphores
----------

The Vulkan API requires semaphores to synchronize access to
avoid rendering corruption. The main rendering loop will
require a pair of semaphores for each swapchain image. The
acquire semaphore is initialized by vkAcquireNextImageKHR()
and consumed by vkQueueSubmit() to ensure the buffer is not
overwritten while it is still presented to the display. The
submit semaphore is initialized by vkQueueSubmit() and
consumed by vkQueuePresentKHR to ensure that all commands
submitted have been processed before a buffer is presented
to the display. Note that there are other uses for
semaphores besides those used by Gears VK but are not
explained here. See the Drawing Commands section for more
details.

	int i;
	for(i = 0; i < swapchain_image_count; ++i)
	{
		VkSemaphoreCreateInfo sa_info =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0
		};

		if(vkCreateSemaphore(device, &sa_info, NULL,
		                     &semaphore_acquire[i]) != VK_SUCCESS)

		VkSemaphoreCreateInfo ss_info =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0
		};

		if(vkCreateSemaphore(device, &ss_info, NULL,
		                     &semaphore_submit[i]) != VK_SUCCESS)
	}

Shaders
-------

There are several changes required to port shaders from
OpenGL to Vulkan. The qualifiers for attribute/varying
have been depreciated in favor of in/out. Additionally, the
shader must also declare layout qualifiers which affect the
storage location of their data and the locations must match
the values specified in Vulkan API calls. The vertex
attributes (vertex and normal) must specify a location
index and this value must match the
VkVertexInputAttributeDescription described in the Graphics
Pipeline section. The uniform variables are declared in a
packed uniform block. The standard packing layout defined by
std140 is used which is cross-platform and cross-vendor.
The ARB_uniform_buffer_object extension describes the std140
layout rules in the Standard Uniform Block section and
provides a number of examples. The uniform block set number
must match the index in vkCmdBindDescriptorSets and the
binding number must match the VkDescriptorSetLayoutBinding.
As I understand, it may be better to use multiple
descriptor sets when the uniform buffer data is updated at
a different frequency as this may impact performance. For
example, the color in set 0 and the MVP/NM in set 1 since
they are updated at different frequencies.

	#version 450

	layout(location=0) in vec3 vertex;
	layout(location=1) in vec3 normal;

	layout(std140, set=0, binding=0) uniform uniformMvp
	{
		mat4 mvp;
	};

	layout(std140, set=0, binding=1) uniform uniformNm
	{
		mat4 nm;
	};

	layout(location=0) out vec3 varying_normal;

	void main()
	{
		varying_normal = normalize(mat3(nm)*normal);
		gl_Position    = mvp*vec4(vertex, 1.0);
	}

The fragment shader must also declare an output variable
for the fragColor whereas OpenGL allows the app to use the
gl_FragColor built-in variable. And finally, the precision
qualifiers are now supported in all versions but default to
highp for desktop versions. See the GL_KHR_vulkan_glsl
extension for more details.

	#version 450

	layout(location=0) in vec3 varying_normal;

	layout(std140, set=0, binding=2) uniform uniformColor
	{
		vec4 color;
	};

	layout(location=0) out vec4 fragColor;

	void main()
	{
		vec4 ambient        = vec4(0.2, 0.2, 0.2, 1.0);
		vec3 light_position = vec3(5.0, 5.0, 10.0);
		light_position      = normalize(light_position);

		float ndotlp  = dot(varying_normal, light_position);
		if(ndotlp > 0.0)
		{
			vec4 diffuse = vec4(ndotlp, ndotlp, ndotlp, 0.0);
			fragColor    = color*(ambient + diffuse);
		}
		else
		{
			fragColor = color*ambient;
		}
	}

As described in the Introduction section, the shaders are
compiled into the SPIR-V intermediate binary format.

	glslangValidator -V shader.vert -o vert.spv
	glslangValidator -V shader.frag -o frag.spv

Shader Modules
--------------

A shader module is simply an object that holds the shader
binary code as compiled by the SPIR-V compiler. See the
Shaders section for more details.

	VkShaderModuleCreateInfo sm_info_vert =
	{
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext    = NULL,
		.flags    = 0,
		.codeSize = size_vert,
		.pCode    = code_vert
	};

	VkShaderModule module_vert;
	if(vkCreateShaderModule(device, &sm_info_vert, NULL,
	                        &module_vert) != VK_SUCCESS)

	VkShaderModuleCreateInfo sm_info_frag =
	{
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext    = NULL,
		.flags    = 0,
		.codeSize = size_frag,
		.pCode    = code_frag
	};

	VkShaderModule module_frag;
	if(vkCreateShaderModule(device, &sm_info_frag, NULL,
	                        &module_frag) != VK_SUCCESS)

Descriptor Set Layout
---------------------

The descriptor set layout is derived from the uniform
variables declared in your shaders. The binding number
should match the value defined in the shader and the
stageFlags indicate all stages the uniform may be accessed
from. See the Shaders section for more details.

	VkDescriptorSetLayoutBinding bindings[3] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		{
			.binding            = 0,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		{
			.binding            = 1,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		{
			.binding            = 2,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}
	};

	VkDescriptorSetLayoutCreateInfo dsl_info =
	{
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext        = NULL,
		.flags        = 0,
		.bindingCount = 3,
		.pBindings    = bindings,
	};

	VkDescriptorSetLayout descriptor_set_layout;
	if(vkCreateDescriptorSetLayout(device,
	                               &dsl_info, NULL,
	                               &descriptor_set_layout) != VK_SUCCESS)

Pipeline Layout
---------------

A pipeline layout describes the complete set of uniform
data that can be accessed by the pipeline. The pipeline
layout consists of descriptor set layouts and push constant
ranges (zero or more of each). A push constant is a uniform
variable in a shader that can be used just like a member of
a uniform block but is declared in the shader with a
push_constant modifier. Push constants are owned by Vulkan
and can be pushed into the pipeline directly from the
command buffer rather than being backed by memory.

	VkPipelineLayoutCreateInfo pl_info =
	{
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext                  = NULL,
		.flags                  = 0,
		.setLayoutCount         = 1,
		.pSetLayouts            = &descriptor_set_layout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges    = NULL
	};

	VkPipelineLayout pipeline_layout;
	if(vkCreatePipelineLayout(device, &pl_info, NULL,
	                          &pipeline_layout) != VK_SUCCESS)

Graphics Pipeline
-----------------

The graphics pipeline construct encapsulates much of the
dynamic state required for rendering. The advantages are
that all encapsulated state can be changed by a single
function call and may enable Vulkan driver level
optimizations which would not be possible when the state
can change independently.

	VkPipelineShaderStageCreateInfo pss_info[2] =
	{
		{
			.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext               = NULL,
			.flags               = 0,
			.stage               = VK_SHADER_STAGE_VERTEX_BIT,
			.module              = module_vert,
			.pName               = "main",
			.pSpecializationInfo = NULL
		},
		{
			.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext               = NULL,
			.flags               = 0,
			.stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module              = module_frag,
			.pName               = "main",
			.pSpecializationInfo = NULL
		}
	};

The binding number is the index of the buffer passed to
vkCmdBindVertexBuffers, the stride is the size of the
primitive, the location was declared in the shader and the
offset is only used for packed vertex buffers.

	// layout(location=0) in vec3 vertex;
	// layout(location=1) in vec3 normal;
	VkVertexInputBindingDescription vib[2] =
	{
		{
			.binding   = 0,
			.stride    = (uint32_t) sizeof(a3d_vec3f_t),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		},
		{
			.binding   = 1,
			.stride    = (uint32_t) sizeof(a3d_vec3f_t),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		}
	};

	// layout(location=0) in vec3 vertex;
	// layout(location=1) in vec3 normal;
	VkVertexInputAttributeDescription via[2] =
	{
		{
			.location = 0,
			.binding  = 0,
			.format   = VK_FORMAT_R32G32B32_SFLOAT,
			.offset   = 0
		},
		{
			.location = 1,
			.binding  = 1,
			.format   = VK_FORMAT_R32G32B32_SFLOAT,
			.offset   = 0
		}
	};

	VkPipelineVertexInputStateCreateInfo pvis_info =
	{
		.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext                           = NULL,
		.flags                           = 0,
		.vertexBindingDescriptionCount   = 2,
		.pVertexBindingDescriptions      = vib,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions    = via
	};

In the Vulkan API you must declare the topology of the vertex
buffer (e.g. triangle list, triangle strip, triangle fan) in
the graphics pipeline rather than when drawing.

	VkPipelineInputAssemblyStateCreateInfo pias_info =
	{
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext                  = NULL,
		.flags                  = 0,
		.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = VK_FALSE
	};

The viewport and scissor are actually ignored because we
specify VK_DYNAMIC_STATE_VIEWPORT and
VK_DYNAMIC_STATE_SCISSOR in the dynamic state. See Surface
Resizes for more details.

	VkViewport viewport =
	{
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float) swapchain_extent.width,
		.height   = (float) swapchain_extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor =
	{
		.offset =
		{
			.x = 0,
			.y = 0
		},
		.extent =
		{
			.width  = (uint32_t) swapchain_extent.width,
			.height = (uint32_t) swapchain_extent.height,
		}
	};

	VkPipelineViewportStateCreateInfo pvs_info =
	{
		.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext         = NULL,
		.flags         = 0,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor
	};

You can easly switch from filled rendering to wireframe
rendering by changing the polygonMode if supported by
the physical device.

	VkPipelineRasterizationStateCreateInfo prs_info =
	{
		.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext                   = NULL,
		.flags                   = 0,
		.depthClampEnable        = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode             = VK_POLYGON_MODE_FILL,
		.cullMode                = VK_CULL_MODE_NONE,
		.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable         = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp          = 0.0f,
		.depthBiasSlopeFactor    = 0.0f,
		.lineWidth               = 1.0f
	};

The Gears VK app does not use multisampling.

	VkPipelineMultisampleStateCreateInfo pms_info =
	{
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable   = VK_FALSE,
		.minSampleShading      = 0.0f,
		.pSampleMask           = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable      = VK_FALSE
	};

Enable depth buffers and disable stencil buffers

	VkPipelineDepthStencilStateCreateInfo pdss_info =
	{
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.depthTestEnable       = VK_TRUE,
		.depthWriteEnable      = VK_TRUE,
		.depthCompareOp        = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable     = VK_FALSE,
		.front =
		{
			.failOp      = VK_STENCIL_OP_KEEP,
			.passOp      = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp   = VK_COMPARE_OP_NEVER,
			.compareMask = 0,
			.writeMask   = 0,
			.reference   = 0
		},
		.back =
		{
			.failOp      = VK_STENCIL_OP_KEEP,
			.passOp      = VK_STENCIL_OP_KEEP,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp   = VK_COMPARE_OP_NEVER,
			.compareMask = 0,
			.writeMask   = 0,
			.reference   = 0
		},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};

Don't accidentally set the colorWriteMask to zero othwise
your rendering will be discarded.

	VkPipelineColorBlendAttachmentState pcbs =
	{
		.blendEnable         = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp        = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp        = VK_BLEND_OP_ADD,
		.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT |
		                       VK_COLOR_COMPONENT_G_BIT |
		                       VK_COLOR_COMPONENT_B_BIT |
		                       VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo pcbs_info =
	{
		.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext           = NULL,
		.flags           = 0,
		.logicOpEnable   = VK_FALSE,
		.logicOp         = VK_LOGIC_OP_CLEAR,
		.attachmentCount = 1,
		.pAttachments    = &pcbs,
		.blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f }
	};

Set the viewport and scissor dynamic state to make resizes
easier. If you do not include this dynamic state than you
will need to recreate the Graphics Pipeline on resizes since
the viewport and scissor defined above will not be ignored.
See the Surface Resizes secton for more details.

	VkDynamicState dynamic_state[2] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo pds_info =
	{
		.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext             = NULL,
		.flags             = 0,
		.dynamicStateCount = 2,
		.pDynamicStates    = dynamic_state,
	};

	VkGraphicsPipelineCreateInfo gp_info =
	{
		.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext               = NULL,
		.flags               = 0,
		.stageCount          = 2,
		.pStages             = pss_info,
		.pVertexInputState   = &pvis_info,
		.pInputAssemblyState = &pias_info,
		.pTessellationState  = NULL,
		.pViewportState      = &pvs_info,
		.pRasterizationState = &prs_info,
		.pMultisampleState   = &pms_info,
		.pDepthStencilState  = &pdss_info,
		.pColorBlendState    = &pcbs_info,
		.pDynamicState       = &pds_info,
		.layout              = pipeline_layout,
		.renderPass          = render_pass,
		.subpass             = 0,
		.basePipelineHandle  = VK_NULL_HANDLE,
		.basePipelineIndex   = -1
	};

	if(vkCreateGraphicsPipelines(device,
	                             pipeline_cache,
	                             1, &gp_info, NULL,
	                             &pipeline) != VK_SUCCESS)

Uniforms and Attributes
-----------------------

The process to create uniform/attribute buffers is nearly
identical except for the usage flag. For uniforms it
should be VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT and for the
vertex attributes it should be
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT. The uniform/attribute
buffers require Vulkan buffer and device memory constructs.

	VkBuffer       buffer;
	VkDeviceMemory memory;

The following steps are required to inialize the buffer and
memory.

	VkBufferCreateInfo b_info =
	{
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = size,
		.usage                 = usage,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &queue_family_index
	};

	if(vkCreateBuffer(device, &b_info, NULL,
	                  &buffer) != VK_SUCCESS)

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(device,
	                              buffer,
	                              &mr);

The following HOST flags are needed to map the buffer in
order to fill the uniform/attribute data. See the
getMemoryTypeIndex section for a utility which selects a
memory type.

	VkFlags mp_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t mt_index;
	if(getMemoryTypeIndex(mr.memoryTypeBits,
	                      mp_flags, &mt_index) == 0)

	VkMemoryAllocateInfo ma_info =
	{
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext           = NULL,
		.allocationSize  = mr.size,
		.memoryTypeIndex = mt_index
	};

	if(vkAllocateMemory(device, &ma_info, NULL,
	                    &memory) != VK_SUCCESS)

Optionally fill the uniform/attribute data when creating
the buffer.

	if(buf)
	{
		void* data;
		if(vkMapMemory(device, memory,
		               0, mr.size, 0, &data) != VK_SUCCESS)
		{
			LOGE("vkMapMemory failed");
			goto fail_map;
		}
		memcpy(data, buf, size);
		vkUnmapMemory(device, memory);
	}

	if(vkBindBufferMemory(device, buffer,
	                      memory, 0) != VK_SUCCESS)

This section of code assumed that the uniform or attribute
constant between frames and as such a single buffer/device
memory pair was sufficient. However, if the app updates the
uniform/attribute at any time after creation then there is
a possibility that the buffer/device memory is in use by
the GPU which could result in corruption if updated (e.g.
triple buffered surfaces are common). To avoid corruption
the app should allocate an array of buffer/device memory
pairs whose size matches the swapchain image count and use
the corresponding pair when rendering each frame.

Descriptor Sets
---------------

A descriptor set is the Vulkan API construct which collects
a set of uniforms into a single object so that the app may
quickly switch between different sets. This allows a Vulkan
app to switch all descriptors in one function call as
compared to OpenGL which required many calls to
glUniformXXX().

This section shows how to allocate descriptor sets from the
descriptor pool and using the descriptor set layout defined
earlier. Then the app is able to connect the descriptor set
to the buffers which were allocated earlier. We use an
array of descriptor sets because the MVP/NM are updated per
frame. Notice that the MVP/NM descriptor updates specify
a different buffer while the color descriptor update reuses
the same buffer since the color for an individual gear
is constant.

	VkDescriptorSetAllocateInfo ds_info =
	{
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext              = NULL,
		.descriptorPool     = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &descriptor_set_layout
	};

	int i;
	for(i = 0; i < swapchain_image_count; ++i)
	{
		if(vkAllocateDescriptorSets(device, &ds_info,
		                            &descriptor_sets[i]) != VK_SUCCESS)

		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		VkDescriptorBufferInfo db_info =
		{
			.buffer  = uniformMvp_buffer[i],
			.offset  = 0,
			.range   = sizeof(a3d_mat4f_t)
		};

		VkWriteDescriptorSet writes =
		{
			.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext            = NULL,
			.dstSet           = descriptor_sets[i],
			.dstBinding       = 0,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo       = NULL,
			.pBufferInfo      = &db_info,
			.pTexelBufferView = NULL,
		};

		vkUpdateDescriptorSets(device, 1, &writes,
		                       0, NULL);

		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		db_info.buffer    = uniformNm_buffer[i];
		db_info.range     = sizeof(a3d_mat4f_t);
		writes.dstBinding = 1;
		vkUpdateDescriptorSets(device, 1, &writes,
		                       0, NULL);

		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		db_info.buffer    = uniformColor_buffer;
		db_info.range     = sizeof(a3d_vec4f_t);
		writes.dstBinding = 2;
		vkUpdateDescriptorSets(device, 1, &writes,
		                       0, NULL);
	}

Drawing Commands
----------------

This section describes the Vulkan API commands required for
the main rendering loop.

Before rendering can occur we must obtain an image from the
swapchain. The image may still be in use by the display
when the image is acquired so the queue submit operation
must be instructed to wait for the acquire semaphore before
it can process the command buffer.

	uint64_t timeout = 250000000;
	if(vkAcquireNextImageKHR(device,
	                         swapchain,
	                         timeout,
	                         semaphore_acquire,
	                         VK_NULL_HANDLE,
	                         &swapchain_frame) != VK_SUCCESS)

Next, we must wait for the swapchain fence which is
signaled when all commands for this swapchain in the
previous frame have completed execution. Once this occurs
we can reset the command buffer for the current frame and
begin submitting commands.

	VkFence swapchain_fence = swapchain_fences[swapchain_frame];
	vkWaitForFences(device, 1,
	                &swapchain_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &swapchain_fence);

	VkCommandBuffer command_buffer;
	command_buffer = command_buffers[swapchain_frame];

	VkFramebuffer framebuffer;
	framebuffer = framebuffers[swapchain_frame];

	if(vkResetCommandBuffer(command_buffer, 0) != VK_SUCCESS)

	VkCommandBufferInheritanceInfo cbi_info =
	{
		.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.pNext                = NULL,
		.renderPass           = render_pass,
		.subpass              = 0,
		.framebuffer          = framebuffer,
		.occlusionQueryEnable = VK_FALSE,
		.queryFlags           = 0,
		.pipelineStatistics   = 0
	};

	VkCommandBufferBeginInfo cb_info =
	{
		.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext            = NULL,
		.flags            = 0,
		.pInheritanceInfo = &cbi_info
	};

	if(vkBeginCommandBuffer(command_buffer,
	                        &cb_info) != VK_SUCCESS)

The next step is to ensure that the color and depth buffers
are in the correct image layout. The color buffer is
initially in the UNDEFINED layout the first time a
swapchain image is acquired and in the PRESENT_SRC_KHR
layout for subsequent frames. Note that the
color_attachment for the subpass declared that it would
use the COLOR_ATTACHMENT_OPTIMAL layout for rendering.
Transitioning to this layout is performed by the following
pipeline barrier.

	VkImageMemoryBarrier imb =
	{
		.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext               = NULL,
		.srcAccessMask       = 0,
		.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = swapchain_images[swapchain_frame],
		.subresourceRange    =
		{
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1
		}
	};

	vkCmdPipelineBarrier(command_buffer,
	                     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	                     0, 0, NULL, 0 ,NULL, 1, &imb);

Similarly, we need to transition the depth buffer from
UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL. However,
this step only needs to be performed for the first frame
the depth image rendered because the renderpass does not
alter the depth image layout for subsequent frames.

	if(depth_transition)
	{

		VkImageMemoryBarrier imb_depth =
		{
			.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext               = NULL,
			.srcAccessMask       = 0,
			.dstAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image               = depth_image,
			.subresourceRange    =
			{
				.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1
			}
		};

		vkCmdPipelineBarrier(command_buffer,
		                     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		                     0, 0, NULL, 0 ,NULL, 1, &imb_depth);

		depth_transition = 0;
	}

The next steps issue commands to set the dymanic state for
the viewport and scissor as explained in the Surface
Resizes section.

	VkViewport viewport =
	{
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float) swapchain_extent.width,
		.height   = (float) swapchain_extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor =
	{
		.offset =
		{
			.x = 0,
			.y = 0
		},
		.extent =
		{
			.width  = (uint32_t) swapchain_extent.width,
			.height = (uint32_t) swapchain_extent.height,
		}
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

The following clear values will be applied when the
renderpass attachment loadOp is set to CLEAR.

	VkClearValue cv[2] =
	{
		{
			.color =
			{
				.float32={ 0.0f, 0.0f, 0.0f, 1.0f }
			},
		},
		{
			.depthStencil =
			{
				.depth   = 1.0f,
				.stencil = 0
			}
		}
	};

	uint32_t width  = swapchain_extent.width;
	uint32_t height = swapchain_extent.height;
	VkRenderPassBeginInfo rp_info =
	{
		.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext           = NULL,
		.renderPass      = render_pass,
		.framebuffer     = framebuffers[swapchain_frame],
		.renderArea      = { { .x=0, .y=0 },
		                     { .width=width,
		                       .height=height } },
		.clearValueCount = 2,
		.pClearValues    = cv
	};

	vkCmdBeginRenderPass(command_buffer,
	                     &rp_info,
	                     VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer,
	                  VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  pipeline);

The shaders uniforms are defined by binding the descriptor
sets.

	vkCmdBindDescriptorSets(command_buffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        pipeline_layout,
	                        0, 1, descriptor_set,
	                        0, NULL);

The shaders vertex attributes are defined by binding of
vertex buffers.

	VkDeviceSize offsets[2] = { 0, 0 };
	VkBuffer     buffers[2] =
	{
		vertex_buffer,
		normal_buffer
	};
	vkCmdBindVertexBuffers(command_buffer, 0, 2,
	                       buffers, offsets);
	vkCmdDraw(command_buffer, vc, 1, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	vkEndCommandBuffer(command_buffer);

Once the command buffer is complete we may submit it to
the device queue for processing by the GPU. The queue will
signal the submit semaphore and the swapchain fence when
the processing is complete.

	VkPipelineStageFlags wait_dst_stage_mask;
	wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo s_info =
	{
		.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext                = NULL,
		.waitSemaphoreCount   = 1,
		.pWaitSemaphores      = &semaphore_acquire,
		.pWaitDstStageMask    = &wait_dst_stage_mask,
		.commandBufferCount   = 1,
		.pCommandBuffers      = &command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores    = &semaphore_submit
	};

	if(vkQueueSubmit(queue, 1, &s_info,
	                 swapchain_fence) != VK_SUCCESS)

And finally we request to display the swapchain image once
the device queue signals the submit semaphore to indicate
it has completed processing of the command buffer.

	VkPresentInfoKHR p_info =
	{
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext              = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &semaphore_submit,
		.swapchainCount     = 1,
		.pSwapchains        = &swapchain,
		.pImageIndices      = &swapchain_frame,
		.pResults           = NULL
	};

	if(vkQueuePresentKHR(queue,
	                     &p_info) != VK_SUCCESS)

Surface Resizes
---------------

The Gears VK app handles resizes in Vulkan by waiting for
the device to idle then it recreates the Vulkan state which
depends on the surface size. This includes the depth buffer,
framebuffer and swapchain. When recreating the swapchain, it
is important to ensure that the swapchain image count
remains the same as the previous surface. This is because
there are many Vulkan object arrays that were allocated to
match the swapchain image count (e.g. uniform buffers,
descriptor sets, command buffers and semaphores).

The graphics pipeline viewport state also depends on the
surface size for the viewport and scissor. To avoid
recreating the graphics pipeline with the new values,
simply add VK_DYNAMIC_STATE_VIEWPORT and
VK_DYNAMIC_STATE_SCISSOR to the dynamic state. And call
the vkCmdSetViewport() and vkCmdSetScissor() functions
prior to beginning the renderpass. See the Graphics
Pipeline and Drawing Commands sections for more details.

On Android be sure to enable the orientation config change
in the AndroidManifest.xml file to enable window resizes.

	android:configChanges="orientation"

See the Main section for detecting surface resizes.

getMemoryTypeIndex
------------------

The getMemoryTypeIndex() utility function queries the
physical device memory properties then iterates over the
memoryTypes to find a suitable match for the requested
memory type bits and memory property flags.

	int getMemoryTypeIndex(uint32_t mt_bits,
                           VkFlags mp_flags,
                           uint32_t* mt_index)
	{
		VkPhysicalDeviceMemoryProperties mp;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &mp);

		int i;
		for(i = 0; i < mp.memoryTypeCount; ++i)
		{
			if(mt_bits & 1)
			{
				VkFlags mp_flagsi;
				mp_flagsi = mp.memoryTypes[i].propertyFlags;
				if((mp_flagsi & mp_flags) == mp_flags)
				{
					*mt_index = i;
					return 1;
				}
			}
			mt_bits >>= 1;
		}

		return 0;
	}

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
