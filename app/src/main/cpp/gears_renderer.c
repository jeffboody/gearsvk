/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Gears for Android is a heavily modified port of the famous "gears" demo.
 * As such, it is a derived work subject to the license requirements (below)
 * of the original work.
 *
 * Copyright (c) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "gears_renderer.h"
#include "a3d/widget/a3d_key.h"
#include "a3d/a3d_timestamp.h"

#define LOG_TAG "gears"
#include "a3d/a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

// gear colors
static const a3d_vec4f_t RED   = { .r=0.8f, .g=0.1f, .b=0.0f, .a=1.0f };
static const a3d_vec4f_t GREEN = { .r=0.0f, .g=0.8f, .b=0.2f, .a=1.0f };
static const a3d_vec4f_t BLUE  = { .r=0.2f, .g=0.2f, .b=1.0f, .a=1.0f };

static void gears_renderer_exit(gears_renderer_t* self)
{
	assert(self);

	gears_renderer_cmd_fn cmd_fn = self->cmd_fn;
	(*cmd_fn)(GEARS_CMD_EXIT, "");
}

static void gears_renderer_step(gears_renderer_t* self)
{
	assert(self);

	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
	// Vulkan uses a top-left left origin while OpenGL
	// uses a bottom-left origin so the frustum top and
	// bottom should be swapped to compensate
	float w = (float) self->swapchain_extent.width;
	float h = (float) self->swapchain_extent.height;
	if(h > w)
	{
		float a = h / w;
		a3d_mat4f_frustum(&self->pm, 1, -1.0f, 1.0f, a, -a, 5.0f, 60.0f);
	}
	else
	{
		float a = w / h;
		a3d_mat4f_frustum(&self->pm, 1, -a, a, 1.0f, -1.0f, 5.0f, 60.0f);
	}
	a3d_mat4f_translate(&self->mvm, 1, 0.0f, 0.0f, -40.0f);

	// glxgears: event_loop
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_scale(&self->mvm, 0, self->view_scale, self->view_scale, self->view_scale);
	a3d_mat4f_rotateq(&self->mvm, 0, &self->view_q);

	// Gear1
	a3d_mat4f_t mvp;
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, -3.0f, -2.0f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, self->angle, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear1, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear2
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, 3.1f, -2.0f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 9.0f, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear2, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear3
	a3d_stack4f_push(self->mvm_stack, &self->mvm);
	a3d_mat4f_translate(&self->mvm, 0, -3.1f, 4.2f, 0.0f);
	a3d_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 25.0f, 0.0f, 0.0f, 1.0f);
	a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear3, &mvp, &self->mvm);
	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	a3d_stack4f_pop(self->mvm_stack, &self->mvm);

	double t     = a3d_timestamp();
	double dt0   = t - self->t0;
	++self->frames;

	// don't update fps every frame
	if(dt0 >= 1.0)
	{
		float seconds  = (float) dt0;
		self->last_fps = ((float) self->frames)/seconds;

		//LOGI("%i frames in %.2lf seconds = %.2lf FPS", self->frames, seconds, self->last_fps);

		self->t0     = t;
		self->frames = 0;
	}

	// next frame
	if(self->t_last > 0.0)
	{
		float dt_last = (float) (t - self->t_last);

		if(self->last_fps > 30.0f)
		{
			dt_last = 1.0f/self->last_fps;
		}

		// make the gears rotate at a constant rate
		// red gear rotates at 1 revolution per 7 seconds
		self->angle += 360.0f * dt_last / 7.0f;
		if(self->angle >= 360.0f)
		{
			self->angle -= 360.0f;
		}
	}
	self->t_last = t;
}

static void gears_renderer_rotate(gears_renderer_t* self,
                                  float dx, float dy)
{
	assert(self);

	// rotating around x-axis is equivalent to moving up-and-down on touchscreen
	// rotating around y-axis is equivalent to moving left-and-right on touchscreen
	// 360 degrees is equivalent to moving completly across the touchscreen
	float   w  = (float) self->swapchain_extent.width;
	float   h  = (float) self->swapchain_extent.height;
	GLfloat rx = 360.0f * dy / h;
	GLfloat ry = 360.0f * dx / w;
	a3d_quaternion_t q;
	a3d_quaternion_loadeuler(&q, rx, ry, 0.0f);
	a3d_quaternion_rotateq(&self->view_q, &q);
}

static void gears_renderer_scale(gears_renderer_t* self,
                                 float scale)
{
	assert(self);

	// scale range
	float min = 0.25f;
	float max = 2.0f;

	self->view_scale *= scale;
	if(self->view_scale < min)  self->view_scale = min;
	if(self->view_scale >= max) self->view_scale = max;
}

static int
gears_renderer_hasDeviceExtensions(gears_renderer_t* self,
                                   uint32_t count,
                                   const char** names)
{
	assert(self);
	assert(count > 0);
	assert(names);

	uint32_t pCount = 0;
	if(vkEnumerateDeviceExtensionProperties(self->physical_device, NULL,
	                                        &pCount, NULL) != VK_SUCCESS)
	{
		LOGE("vkEnumerateDeviceExtensionProperties failed");
		return 0;
	}

	VkExtensionProperties* properties;
	properties = (VkExtensionProperties*)
	             calloc(pCount, sizeof(VkExtensionProperties));
	if(properties == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	if(vkEnumerateDeviceExtensionProperties(self->physical_device, NULL,
	                                        &pCount, properties) != VK_SUCCESS)
	{
		LOGE("vkEnumerateDeviceExtensionProperties failed");
		goto fail_properties;
	}

	// log device extensions
	int i;
	LOGI("Vulkan Device Extensions:");
	for(i = 0; i < pCount; ++i)
	{
		LOGI("%i: %s", i, properties[i].extensionName);
	}

	// check for enabled extensions
	for(i = 0; i < count; ++i)
	{
		int found = 0;
		int j;
		for(j = 0; j < pCount; ++j)
		{
			if(strcmp(names[i],
			          properties[j].extensionName) == 0)
			{
				found = 1;
				break;
			}
		}

		if(found == 0)
		{
			LOGE("%s not found", names[i]);
			goto fail_enabled;
		}
	}

	free(properties);

	// success
	return 1;

	// failure
	fail_enabled:
	fail_properties:
		free(properties);
	return 0;
}

#ifndef ANDROID
static int
gears_renderer_initSDL(gears_renderer_t* self,
                       const char* app_name)
{
	assert(self);
	assert(app_name);

	SDL_version version;
	SDL_VERSION(&version);
	LOGI("SDL %i.%i.%i", version.major, version.minor, version.patch);

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOGE("SDL_Init failed %s", SDL_GetError());
		return 0;
	}

	if(SDL_GetNumVideoDisplays() <= 0)
	{
		LOGE("SDL_GetNumVideoDisplays failed %s",
		     SDL_GetError());
		return 0;
	}

	// set the default screen size
	int width      = 1920;
	int height     = 1080;
	int fullscreen = 1;
	SDL_DisplayMode dpy;
	if(SDL_GetCurrentDisplayMode(0, &dpy) == 0)
	{
		width  = dpy.w;
		height = dpy.h;
	}

	// override the default screen size
	FILE* f = fopen("sdl.cfg", "r");
	if(f)
	{
		if(fscanf(f, "%i %i %i",
		          &width, &height, &fullscreen) != 3)
		{
			LOGW("fscanf failed");
		}
		fclose(f);
	}

	int flags = (fullscreen ? SDL_WINDOW_FULLSCREEN : 0) |
		        SDL_WINDOW_RESIZABLE |
	            SDL_WINDOW_VULKAN    |
	            SDL_WINDOW_SHOWN;
	self->window = SDL_CreateWindow(app_name,
	                                SDL_WINDOWPOS_UNDEFINED,
	                                SDL_WINDOWPOS_UNDEFINED,
	                                width, height, flags);
	if(self->window == NULL)
	{
		LOGE("SDL_CreateWindow failed: %s", SDL_GetError());
		goto fail_window;
	}

	// success
	return 1;

	// failure
	fail_window:
		SDL_Quit();
	return 0;
}
#endif // ANDROID

static int
gears_renderer_createInstance(gears_renderer_t* self,
                              const char* app_name,
                              uint32_t app_version)
{
	assert(self);
	assert(app_name);

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

	VkApplicationInfo a_info =
	{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext              = NULL,
		.pApplicationName   = app_name,
		.applicationVersion = app_version,
		.pEngineName        = app_name,
		.engineVersion      = app_version,
		.apiVersion         = VK_MAKE_VERSION(1,0,0),
	};

	#ifndef ANDROID
		#define DEBUG_LAYERS
	#endif
	#ifdef DEBUG_LAYERS
		uint32_t layer_count = 1;
		const char* layer_names[] =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};
	#else
		uint32_t layer_count = 0;
		const char** layer_names = NULL;
	#endif
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

	if(vkCreateInstance(&ic_info, NULL,
	                    &self->instance) != VK_SUCCESS)
	{
		LOGE("vkCreateInstance failed");
		return 0;
	}

	return 1;
}

static int
gears_renderer_createSurface(gears_renderer_t* self)
{
	assert(self);

	#ifdef ANDROID
		VkAndroidSurfaceCreateInfoKHR as_info =
		{
			.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.pNext  = NULL,
			.flags  = 0,
			.window = self->app->window
		};

		if(vkCreateAndroidSurfaceKHR(self->instance,
		                             &as_info, NULL,
		                             &self->surface) != VK_SUCCESS)
		{
			LOGE("vkCreateAndroidSurfaceKHR failed");
			return 0;
		}
	#else
		if(SDL_Vulkan_CreateSurface(self->window, self->instance,
		                            &self->surface) == SDL_FALSE)
		{
			LOGE("SDL_Vulkan_CreateSurface failed: %s", SDL_GetError());
			return 0;
		}
	#endif

	return 1;
}

static int
gears_renderer_getPhysicalDevice(gears_renderer_t* self)
{
	assert(self);

	uint32_t physical_device_count;
	if(vkEnumeratePhysicalDevices(self->instance,
	                              &physical_device_count,
	                              NULL) != VK_SUCCESS)
	{
		LOGE("vkEnumeratePhysicalDevices failed");
		return 0;
	}

	if(physical_device_count < 1)
	{
		LOGE("physical_device_count=%u", physical_device_count);
		return 0;
	}

	// select the first physical device
	VkResult result;
	physical_device_count = 1;
	result = vkEnumeratePhysicalDevices(self->instance,
	                                    &physical_device_count,
	                                    &self->physical_device);
	if((result == VK_SUCCESS) || (result == VK_INCOMPLETE))
	{
		// ok
	}
	else
	{
		LOGE("vkEnumeratePhysicalDevices failed");
		return 0;
	}

	return 1;
}

static int
gears_renderer_createDevice(gears_renderer_t* self)
{
	assert(self);

	uint32_t    extension_count   = 1;
	const char* extension_names[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	if(gears_renderer_hasDeviceExtensions(self,
	                                      extension_count,
	                                      extension_names) == 0)
	{
		return 0;
	}

	uint32_t qfp_count;
	vkGetPhysicalDeviceQueueFamilyProperties(self->physical_device,
	                                         &qfp_count,
	                                         NULL);

	VkQueueFamilyProperties* qfp;
	qfp = (VkQueueFamilyProperties*)
	      calloc(qfp_count,
	             sizeof(VkQueueFamilyProperties));
	if(qfp == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	vkGetPhysicalDeviceQueueFamilyProperties(self->physical_device,
	                                         &qfp_count,
	                                         qfp);

	int i;
	int has_index = 0;
	self->queue_family_index = 0;
	for(i = 0; i < qfp_count; ++i)
	{
		if(qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VkBool32 supported;
			if(vkGetPhysicalDeviceSurfaceSupportKHR(self->physical_device,
			                                        i, self->surface,
			                                        &supported) != VK_SUCCESS)
			{
				LOGW("vkGetPhysicalDeviceSurfaceSupportKHR failed");
				continue;
			}

			if(supported && (has_index == 0))
			{
				// select the first supported device queue
				self->queue_family_index = i;
				has_index = 1;
			}
		}
	}

	if(has_index == 0)
	{
		LOGE("vkGetPhysicalDeviceQueueFamilyProperties failed");
		goto fail_select_qfp;
	}

	float queue_priority = 0.0f;
	VkDeviceQueueCreateInfo dqc_info =
	{
		.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext            = NULL,
		.flags            = 0,
		.queueFamilyIndex = self->queue_family_index,
		.queueCount       = 1,
		.pQueuePriorities = &queue_priority
	};

	VkDeviceCreateInfo dc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount    = 1,
		.pQueueCreateInfos       = &dqc_info,
		.enabledLayerCount       = 0,
		.ppEnabledLayerNames     = NULL,
		.enabledExtensionCount   = extension_count,
		.ppEnabledExtensionNames = extension_names,
		.pEnabledFeatures        = NULL
	};

	if(vkCreateDevice(self->physical_device, &dc_info,
	                  NULL, &self->device) != VK_SUCCESS)
	{
		LOGE("vkCreateDevice failed");
		goto fail_create_device;
	}

	vkGetDeviceQueue(self->device,
	                 self->queue_family_index,
                     0, &self->queue);

	free(qfp);

	// success
	return 1;

	// failure
	fail_create_device:
	fail_select_qfp:
		free(qfp);
	return 0;
}

static int
gears_renderer_createCacheAndPools(gears_renderer_t* self)
{
	assert(self);

	VkPipelineCacheCreateInfo pc_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData    = NULL
	};

	if(vkCreatePipelineCache(self->device, &pc_info, NULL,
	                         &self->pipeline_cache) != VK_SUCCESS)
	{
		LOGE("vkCreatePipelineCache failed");
		return 0;
	}

	VkCommandPoolCreateInfo cpc_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = self->queue_family_index
	};

	if(vkCreateCommandPool(self->device, &cpc_info, NULL,
	                       &self->command_pool) != VK_SUCCESS)
	{
		LOGE("vkCreateCommandPool failed");
		goto fail_command_pool;
	}

	// TODO - How to determine descriptorCount and maxSets?
	VkDescriptorPoolSize pool_size =
	{
		.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 64
	};
	VkDescriptorPoolCreateInfo dp_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets       = 64,
		.poolSizeCount = 1,
		.pPoolSizes    = &pool_size
	};

	if(vkCreateDescriptorPool(self->device, &dp_info, NULL,
	                          &self->descriptor_pool) != VK_SUCCESS)
	{
		LOGE("vkCreateDescriptorPool failed");
		goto fail_descriptor_pool;
	}

	// success
	return 1;

	// failure
	fail_descriptor_pool:
		vkDestroyCommandPool(self->device,
		                     self->command_pool, NULL);
	fail_command_pool:
		vkDestroyPipelineCache(self->device,
		                       self->pipeline_cache, NULL);
	return 0;
}

static int
gears_renderer_createSwapchain(gears_renderer_t* self)
{
	assert(self);

	VkSurfaceCapabilitiesKHR caps;
	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(self->physical_device,
	                                             self->surface,
	                                             &caps) != VK_SUCCESS)
	{
		LOGE("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
		return 0;
	}

	// check the minImageCount
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

	uint32_t sf_count;
	if(vkGetPhysicalDeviceSurfaceFormatsKHR(self->physical_device,
	                                        self->surface,
	                                        &sf_count,
	                                        NULL) != VK_SUCCESS)
	{
		LOGE("vkGetPhysicalDeviceSurfaceFormatsKHR failed");
		return 0;
	}

	VkSurfaceFormatKHR* sf;
	sf = (VkSurfaceFormatKHR*)
	     calloc(sf_count, sizeof(VkSurfaceFormatKHR));
	if(sf == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	if(vkGetPhysicalDeviceSurfaceFormatsKHR(self->physical_device,
	                                        self->surface,
	                                        &sf_count,
	                                        sf) != VK_SUCCESS)
	{
		LOGE("vkGetPhysicalDeviceSurfaceFormatsKHR failed");
		goto fail_surface_formats3;
	}

	self->swapchain_frame       = 0;
	self->swapchain_format      = sf[0].format;
	self->swapchain_color_space = sf[0].colorSpace;
	int i;
	for(i = 0; i < sf_count; ++i)
	{
		if((sf[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) &&
		   ((sf[i].format == VK_FORMAT_R8G8B8A8_UNORM) ||
		    (sf[i].format == VK_FORMAT_B8G8R8A8_UNORM)))
		{
			self->swapchain_format      = sf[i].format;
			self->swapchain_color_space = sf[i].colorSpace;
			break;
		}
	}

	uint32_t pm_count;
	if(vkGetPhysicalDeviceSurfacePresentModesKHR(self->physical_device,
	                                             self->surface,
	                                             &pm_count,
	                                             NULL) != VK_SUCCESS)
	{
		goto fail_present_modes1;
	}

	VkPresentModeKHR* pm;
	pm = (VkPresentModeKHR*)
	     calloc(pm_count, sizeof(VkPresentModeKHR));
	if(pm == NULL)
	{
		goto fail_present_modes2;
	}

	if(vkGetPhysicalDeviceSurfacePresentModesKHR(self->physical_device,
	                                             self->surface,
	                                             &pm_count,
	                                             pm) != VK_SUCCESS)
	{
		goto fail_present_modes3;
	}

	VkPresentModeKHR present_mode = pm[0];
	for(i = 0; i < pm_count; ++i)
	{
		if(pm[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			present_mode = pm[i];
			break;
		}
	}

	VkSurfaceTransformFlagBitsKHR preTransform;
	if(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = caps.currentTransform;
	}

	VkSwapchainCreateInfoKHR sc_info =
	{
		.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext   = NULL,
		.flags   = 0,
		.surface = self->surface,
		.minImageCount         = minImageCount,
		.imageFormat           = self->swapchain_format,
		.imageColorSpace       = self->swapchain_color_space,
		.imageExtent           = caps.currentExtent,
		.imageArrayLayers      = 1,
		.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &self->queue_family_index,
		.preTransform          = preTransform,
		.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode           = present_mode,
		.clipped               = VK_TRUE,
		.oldSwapchain          = VK_NULL_HANDLE
	};

	self->swapchain_extent = caps.currentExtent;

	if(vkCreateSwapchainKHR(self->device, &sc_info, NULL,
	                        &self->swapchain) != VK_SUCCESS)
	{
		LOGE("vkCreateSwapchainKHR failed");
		goto fail_swapchain;
	}

	uint32_t count = 0;
	if(vkGetSwapchainImagesKHR(self->device,
	                           self->swapchain,
	                           &count,
	                           NULL) != VK_SUCCESS)
	{
		LOGE("vkGetSwapchainImagesKHR failed");
		goto fail_get1;
	}

	// validate swapchain_image_count across resizes
	if(self->swapchain_image_count &&
	   (self->swapchain_image_count != count))
	{
		LOGE("invalid %u, %u",
		     self->swapchain_image_count, count);
		goto fail_count;
	}
	self->swapchain_image_count = count;

	self->swapchain_images = (VkImage*)
	                         calloc(self->swapchain_image_count,
	                                sizeof(VkImage));
	if(self->swapchain_images == NULL)
	{
		LOGE("calloc failed");
		goto fail_images;
	}

	self->swapchain_fences = (VkFence*)
	                         calloc(self->swapchain_image_count,
	                                sizeof(VkFence));
	if(self->swapchain_fences == NULL)
	{
		LOGE("calloc failed");
		goto fail_fences;
	}

	uint32_t image_count = self->swapchain_image_count;
	if(vkGetSwapchainImagesKHR(self->device,
	                           self->swapchain,
	                           &image_count,
	                           self->swapchain_images) != VK_SUCCESS)
	{
		LOGE("vkGetSwapchainImagesKHR failed");
		goto fail_get2;
	}

	for(i = 0; i < image_count; ++i)
	{
		VkFenceCreateInfo f_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		if(vkCreateFence(self->device, &f_info, NULL,
		                 &self->swapchain_fences[i]) != VK_SUCCESS)
		{
			goto fail_create_fence;
		}
	}

	free(pm);
	free(sf);

	// success
	return 1;

	// failure
	fail_create_fence:
	{
		int j;
		for(j = 0; j < i; ++j)
		{
			vkDestroyFence(self->device,
			               self->swapchain_fences[j], NULL);
		}
	}
	fail_get2:
		free(self->swapchain_fences);
	fail_fences:
		free(self->swapchain_images);
	fail_images:
	fail_count:
	fail_get1:
		vkDestroySwapchainKHR(self->device,
		                      self->swapchain, NULL);
	fail_swapchain:
	fail_present_modes3:
		free(pm);
	fail_present_modes2:
	fail_present_modes1:
	fail_surface_formats3:
		free(sf);
	return 0;
}

static void
gears_renderer_destroySwapchain(gears_renderer_t* self)
{
	assert(self);

	if(self->swapchain == VK_NULL_HANDLE)
	{
		return;
	}

	int i;
	for(i = 0; i < self->swapchain_image_count; ++i)
	{
		vkDestroyFence(self->device,
		               self->swapchain_fences[i], NULL);
	}
	free(self->swapchain_fences);
	free(self->swapchain_images);
	vkDestroySwapchainKHR(self->device,
	                      self->swapchain, NULL);
	self->swapchain_fences = NULL;
	self->swapchain_images = NULL;
	self->swapchain        = VK_NULL_HANDLE;
}

static int
gears_renderer_createDepth(gears_renderer_t* self)
{
	assert(self);

	VkImageCreateInfo i_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType   = VK_IMAGE_TYPE_2D,
		.format      = VK_FORMAT_D16_UNORM,
		.extent      =
		{
			self->swapchain_extent.width,
			self->swapchain_extent.height,
			1
		},
		.mipLevels   = 1,
		.arrayLayers = 1,
		.samples     = VK_SAMPLE_COUNT_1_BIT,
		.tiling      = VK_IMAGE_TILING_OPTIMAL,
		.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &self->queue_family_index,
		.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
	};

	if(vkCreateImage(self->device, &i_info, NULL,
	                 &self->depth_image) != VK_SUCCESS)
	{
		LOGE("vkCreateImage failed");
		return 0;
	}

	VkMemoryRequirements mr;
	vkGetImageMemoryRequirements(self->device,
	                             self->depth_image,
	                             &mr);

	VkFlags  mp_flags = 0;
	uint32_t mt_index;
	if(gears_renderer_getMemoryTypeIndex(self,
	                                     mr.memoryTypeBits,
	                                     mp_flags,
	                                     &mt_index) == 0)
	{
		goto fail_memory_type;
	}

	VkMemoryAllocateInfo ma_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize  = mr.size,
		.memoryTypeIndex = mt_index
	};

	if(vkAllocateMemory(self->device, &ma_info, NULL,
	                    &self->depth_memory) != VK_SUCCESS)
	{
		LOGE("vkAllocateMemory failed");
		goto fail_allocate;
	}

	if(vkBindImageMemory(self->device,
	                     self->depth_image,
	                     self->depth_memory,
	                     0) != VK_SUCCESS)
	{
		LOGE("vkBindBufferMemory failed");
		goto fail_bind;
	}

	VkImageViewCreateInfo iv_info =
	{
		.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext      = NULL,
		.flags      = 0,
		.image      = self->depth_image,
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

	if(vkCreateImageView(self->device, &iv_info, NULL,
	                     &self->depth_image_view) != VK_SUCCESS)
	{
		LOGE("vkCreateImageView failed");
		goto fail_image_view;
	}

	self->depth_transition = 1;

	// success
	return 1;

	// failure
	fail_image_view:
	fail_bind:
		vkFreeMemory(self->device,
		             self->depth_memory, NULL);
		self->depth_memory = VK_NULL_HANDLE;
	fail_allocate:
	fail_memory_type:
		vkDestroyImage(self->device,
		               self->depth_image, NULL);
		self->depth_image = VK_NULL_HANDLE;
	return 0;
}

static void
gears_renderer_destroyDepth(gears_renderer_t* self)
{
	assert(self);

	if(self->depth_image == VK_NULL_HANDLE)
	{
		return;
	}

	vkDestroyImageView(self->device,
	                   self->depth_image_view,
	                   NULL);
	vkFreeMemory(self->device,
	             self->depth_memory, NULL);
	vkDestroyImage(self->device,
	               self->depth_image, NULL);
	self->depth_image_view = VK_NULL_HANDLE;
	self->depth_memory     = VK_NULL_HANDLE;
	self->depth_image      = VK_NULL_HANDLE;
}

static int
gears_renderer_createFramebuffer(gears_renderer_t* self)
{
	assert(self);

	self->framebuffer_image_views = (VkImageView*)
	                                calloc(self->swapchain_image_count,
	                                       sizeof(VkImageView));
	if(self->framebuffer_image_views == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	self->framebuffers = (VkFramebuffer*)
	                     calloc(self->swapchain_image_count,
	                            sizeof(VkFramebuffer));
	if(self->framebuffers == NULL)
	{
		LOGE("calloc failed");
		goto fail_alloc_framebuffers;
	}

	int i;
	for(i = 0; i < self->swapchain_image_count; ++i)
	{
		VkImageViewCreateInfo iv_info =
		{
			.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext      = NULL,
			.flags      = 0,
			.image      = self->swapchain_images[i],
			.viewType   = VK_IMAGE_VIEW_TYPE_2D,
			.format     = self->swapchain_format,
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

		if(vkCreateImageView(self->device, &iv_info, NULL,
		                     &self->framebuffer_image_views[i]) != VK_SUCCESS)
		{
			LOGE("vkCreateImageView failed");
			goto fail_image_view;
		}

		VkImageView attachments[2] =
		{
			self->framebuffer_image_views[i],
			self->depth_image_view
		};

		VkFramebufferCreateInfo f_info =
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.renderPass = self->render_pass,
			.attachmentCount = 2,
			.pAttachments    = attachments,
			.width           = self->swapchain_extent.width,
			.height          = self->swapchain_extent.height,
			.layers          = 1,
		};

		if(vkCreateFramebuffer(self->device, &f_info, NULL,
		                       &self->framebuffers[i]) != VK_SUCCESS)
		{
			vkDestroyImageView(self->device,
			                   self->framebuffer_image_views[i],
			                   NULL);
			self->framebuffer_image_views[i] = VK_NULL_HANDLE;

			LOGE("vkCreateFramebuffer failed");
			goto fail_framebuffer;
		}
	}

	// success
	return 1;

	// failure
	fail_framebuffer:
	fail_image_view:
	{
		int j;
		for(j = 0; j < i; ++j)
		{
			vkDestroyFramebuffer(self->device,
			                     self->framebuffers[j],
			                     NULL);
			vkDestroyImageView(self->device,
			                   self->framebuffer_image_views[j],
			                   NULL);
			self->framebuffers[j]            = VK_NULL_HANDLE;
			self->framebuffer_image_views[j] = VK_NULL_HANDLE;
		}
		free(self->framebuffers);
		self->framebuffers = NULL;
	}
	fail_alloc_framebuffers:
		free(self->framebuffer_image_views);
		self->framebuffer_image_views = NULL;
	return 0;
}

static void
gears_renderer_destroyFramebuffer(gears_renderer_t* self)
{
	assert(self);

	if(self->framebuffers == NULL)
	{
		return;
	}

	int i;
	for(i = 0; i < self->swapchain_image_count; ++i)
	{
		vkDestroyFramebuffer(self->device,
		                     self->framebuffers[i],
		                     NULL);
		vkDestroyImageView(self->device,
		                   self->framebuffer_image_views[i],
		                   NULL);
		self->framebuffers[i]            = VK_NULL_HANDLE;
		self->framebuffer_image_views[i] = VK_NULL_HANDLE;
	}
	free(self->framebuffers);
	free(self->framebuffer_image_views);
	self->framebuffers            = NULL;
	self->framebuffer_image_views = NULL;
}

static int
gears_renderer_createRenderpass(gears_renderer_t* self)
{
	assert(self);

	VkAttachmentDescription attachments[2] =
	{
		{
			.flags          = 0,
			.format         = self->swapchain_format,
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
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = 2,
		.pAttachments    = attachments,
		.subpassCount    = 1,
		.pSubpasses      = &subpass,
		.dependencyCount = 0,
		.pDependencies   = NULL
	};

	if(vkCreateRenderPass(self->device, &rp_info, NULL,
	                      &self->render_pass) != VK_SUCCESS)
	{
		LOGE("vkCreateRenderPass failed");
		return 0;
	}

	return 1;
}

static int
gears_renderer_createCommandBuffers(gears_renderer_t* self)
{
	assert(self);

	self->command_buffers = (VkCommandBuffer*)
	                        calloc(self->swapchain_image_count,
	                               sizeof(VkCommandBuffer));
	if(self->command_buffers == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	VkCommandBufferAllocateInfo cba_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = self->command_pool,
		.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = self->swapchain_image_count
	};

	if(vkAllocateCommandBuffers(self->device, &cba_info,
	                            self->command_buffers) != VK_SUCCESS)
	{
		LOGE("vkAllocateCommandBuffers failed");
		goto fail_allocate;
	}

	// success
	return 1;

	// failure
	fail_allocate:
		free(self->command_buffers);
	return 0;
}

static int
gears_renderer_createSemaphores(gears_renderer_t* self)
{
	assert(self);

	self->semaphore_index   = 0;
	self->semaphore_acquire = (VkSemaphore*)
	                          calloc(self->swapchain_image_count,
	                                 sizeof(VkSemaphore));
	if(self->semaphore_acquire == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	self->semaphore_submit = (VkSemaphore*)
	                         calloc(self->swapchain_image_count,
	                                sizeof(VkSemaphore));
	if(self->semaphore_submit == NULL)
	{
		LOGE("calloc failed");
		goto fail_alloc_ss;
	}

	int i;
	for(i = 0; i < self->swapchain_image_count; ++i)
	{
		VkSemaphoreCreateInfo sa_info =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0
		};

		if(vkCreateSemaphore(self->device, &sa_info, NULL,
		                     &self->semaphore_acquire[i]) != VK_SUCCESS)
		{
			LOGE("vkCreateSemaphore failed");
			goto fail_create;
		}

		VkSemaphoreCreateInfo ss_info =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0
		};

		if(vkCreateSemaphore(self->device, &ss_info, NULL,
		                     &self->semaphore_submit[i]) != VK_SUCCESS)
		{
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);

			LOGE("vkCreateSemaphore failed");
			goto fail_create;
		}
	}

	// success
	return 1;

	// failure
	fail_create:
	{
		int j;
		for(j = 0; j < i; ++j)
		{
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[j],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[j],
			                   NULL);
		}
		free(self->semaphore_submit);
	}
	fail_alloc_ss:
		free(self->semaphore_acquire);
	return 0;
}

static uint32_t*
gears_renderer_importModule(gears_renderer_t* self,
                            const char* fname, size_t* _size)
{
	assert(self);
	assert(fname);
	assert(_size);

	#ifdef ANDROID
		AAssetManager* am;
		am = self->app->activity->assetManager;
		AAsset* f = AAssetManager_open(am, fname,
		                               AASSET_MODE_BUFFER);
		if(f == NULL)
		{
			LOGE("AAssetManager_open %s failed", fname);
			return NULL;
		}

		size_t size = (size_t) AAsset_getLength(f);
		if((size == 0) || ((size % 4) != 0))
		{
			LOGE("invalid size=%u", (unsigned int) size);
			goto fail_size;
		}

		uint32_t* code;
		code = (uint32_t*)
		       calloc(size/4, sizeof(uint32_t));
		if(code == NULL)
		{
			LOGE("calloc failed");
			goto fail_alloc;
		}

		if(AAsset_read(f, code, size) != size)
		{
			LOGE("AAsset_read failed");
			goto fail_code;
		}

		AAsset_close(f);
	#else
		FILE* f = fopen(fname, "r");
		if(f == NULL)
		{
			LOGE("fopen %s failed", fname);
			return NULL;
		}

		fseek(f, 0, SEEK_END);
		size_t size = (size_t) ftell(f);

		if((size == 0) || ((size % 4) != 0))
		{
			LOGE("invalid size=%u", (unsigned int) size);
			goto fail_size;
		}

		if(fseek(f, 0, SEEK_SET) == -1)
		{
			LOGE("fseek failed");
			goto fail_size;
		}

		uint32_t* code;
		code = (uint32_t*)
		       calloc(size/4, sizeof(uint32_t));
		if(code == NULL)
		{
			LOGE("calloc failed");
			goto fail_alloc;
		}

		if(fread((void*) code, size, 1, f) != 1)
		{
			LOGE("fread failed");
			goto fail_code;
		}

		fclose(f);
	#endif

	*_size = size;

	// success
	return code;

	// failure
	fail_code:
		free(code);
	fail_alloc:
	fail_size:
	#ifdef ANDROID
		AAsset_close(f);
	#else
		fclose(f);
	#endif
	return NULL;
}

static int
gears_renderer_createModules(gears_renderer_t* self)
{
	assert(self);

	size_t    size_vert = 0;
	uint32_t* code_vert;
	code_vert = gears_renderer_importModule(self,
	                                        "shaders/vert.spv",
	                                        &size_vert);
	if(code_vert == NULL)
	{
		return 0;
	}

	size_t    size_frag = 0;
	uint32_t* code_frag;
	code_frag = gears_renderer_importModule(self,
	                                        "shaders/frag.spv",
	                                        &size_frag);
	if(code_frag == NULL)
	{
		goto fail_import_frag;
	}

	VkShaderModuleCreateInfo sm_info_vert =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = size_vert,
		.pCode    = code_vert
	};

	if(vkCreateShaderModule(self->device, &sm_info_vert, NULL,
	                        &self->module_vert) != VK_SUCCESS)
	{
		LOGE("vkCreateShaderModule failed");
		goto fail_vert;
	}

	VkShaderModuleCreateInfo sm_info_frag =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = size_frag,
		.pCode    = code_frag
	};

	if(vkCreateShaderModule(self->device, &sm_info_frag, NULL,
	                        &self->module_frag) != VK_SUCCESS)
	{
		LOGE("vkCreateShaderModule failed");
		goto fail_frag;
	}

	free(code_frag);
	free(code_vert);

	// success
	return 1;

	// failure
	fail_frag:
		vkDestroyShaderModule(self->device,
		                      self->module_vert, NULL);
	fail_vert:
		free(code_frag);
	fail_import_frag:
		free(code_vert);
	return 0;
}

static int
gears_renderer_createDescriptorSetLayout(gears_renderer_t* self)
{
	assert(self);

	VkDescriptorSetLayoutBinding binding_uniformBuffer[3] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		{
			.binding = 0,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		{
			.binding = 1,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		{
			.binding = 2,
			.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}
	};

	VkDescriptorSetLayoutCreateInfo dsl_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = 3,
		.pBindings    = binding_uniformBuffer,
	};

	VkDescriptorSetLayout dsl;
	if(vkCreateDescriptorSetLayout(self->device,
	                               &dsl_info, NULL,
	                               &dsl) != VK_SUCCESS)
	{
		LOGE("vkCreateDescriptorSetLayout failed");
		return 0;
	}
	self->descriptor_set_layout = dsl;

	return 1;
}

static int
gears_renderer_createPipelineLayout(gears_renderer_t* self)
{
	assert(self);

	VkPipelineLayoutCreateInfo pl_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts    = &self->descriptor_set_layout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges    = NULL
	};

	if(vkCreatePipelineLayout(self->device, &pl_info, NULL,
	                          &self->pipeline_layout) != VK_SUCCESS)
	{
		LOGE("vkCreatePipelineLayout failed");
		return 0;
	}

	return 1;
}

static int
gears_renderer_createGraphicsPipeline(gears_renderer_t* self)
{
	assert(self);

	VkPipelineShaderStageCreateInfo pss_info[2] =
	{
		// vertex stage
		{
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext  = NULL,
			.flags  = 0,
			.stage  = VK_SHADER_STAGE_VERTEX_BIT,
			.module = self->module_vert,
			.pName  = "main",
			.pSpecializationInfo = NULL
		},

		// fragment stage
		{
			.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext  = NULL,
			.flags  = 0,
			.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = self->module_frag,
			.pName  = "main",
			.pSpecializationInfo = NULL
		}
	};

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
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount   = 2,
		.pVertexBindingDescriptions      = vib,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions    = via
	};

	VkPipelineInputAssemblyStateCreateInfo pias_info =
	{
		.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext    = NULL,
		.flags    = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport =
	{
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float) self->swapchain_extent.width,
		.height   = (float) self->swapchain_extent.height,
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
			.width  = (uint32_t) self->swapchain_extent.width,
			.height = (uint32_t) self->swapchain_extent.height,
		}
	};

	VkPipelineViewportStateCreateInfo pvs_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor
	};

	VkPipelineRasterizationStateCreateInfo prs_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
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

	VkPipelineMultisampleStateCreateInfo pms_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable   = VK_FALSE,
		.minSampleShading      = 0.0f,
		.pSampleMask           = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable      = VK_FALSE
	};

	VkPipelineDepthStencilStateCreateInfo pdss_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthTestEnable        = VK_TRUE,
		.depthWriteEnable       = VK_TRUE,
		.depthCompareOp         = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable  = VK_FALSE,
		.stencilTestEnable      = VK_FALSE,
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

	VkPipelineColorBlendAttachmentState pcbs =
	{
		.blendEnable         = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp        = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp        = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		                  VK_COLOR_COMPONENT_G_BIT |
		                  VK_COLOR_COMPONENT_B_BIT |
		                  VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo pcbs_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable   = VK_FALSE,
		.logicOp         = VK_LOGIC_OP_CLEAR,
		.attachmentCount = 1,
		.pAttachments    = &pcbs,
		.blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	VkDynamicState dynamic_state[2] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo pds_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = 2,
		.pDynamicStates    = dynamic_state,
	};

	VkGraphicsPipelineCreateInfo gp_info =
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
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
		.layout              = self->pipeline_layout,
		.renderPass          = self->render_pass,
		.subpass             = 0,
		.basePipelineHandle  = VK_NULL_HANDLE,
		.basePipelineIndex   = -1
	};

	if(vkCreateGraphicsPipelines(self->device,
	                             self->pipeline_cache,
	                             1, &gp_info, NULL,
	                             &self->pipeline) != VK_SUCCESS)
	{
		LOGE("vkCreateGraphicsPipelines failed");
		return 0;
	}

	return 1;
}

static void
gears_renderer_nextSemaphore(gears_renderer_t* self,
                             VkSemaphore* semaphore_acquire,
                             VkSemaphore* semaphore_submit)
{
	assert(self);
	assert(semaphore_acquire);
	assert(semaphore_submit);

	uint32_t idx       = self->semaphore_index;
	*semaphore_acquire = self->semaphore_acquire[idx];
	*semaphore_submit  = self->semaphore_submit[idx];

	++idx;
	self->semaphore_index = idx%self->swapchain_image_count;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_renderer_t* gears_renderer_new(void* app,
                                     const char* app_name,
                                     uint32_t app_version,
                                     gears_renderer_cmd_fn cmd_fn)
{
	#ifdef ANDROID
		assert(app);
	#else
		assert(app == NULL);
	#endif
	assert(app_name);

	gears_renderer_t* self;
	self = (gears_renderer_t*)
	       calloc(1, sizeof(gears_renderer_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	#ifdef ANDROID
		self->app = (struct android_app*) app;
	#else
		if(gears_renderer_initSDL(self, app_name) == 0)
		{
			free(self);
			return NULL;
		}
	#endif

	if(gears_renderer_createInstance(self, app_name,
	                                 app_version) == 0)
	{
		goto fail_instance;
	}

	if(gears_renderer_createSurface(self) == 0)
	{
		goto fail_surface;
	}

	if(gears_renderer_getPhysicalDevice(self) == 0)
	{
		goto fail_physical_device;
	}

	if(gears_renderer_createDevice(self) == 0)
	{
		goto fail_device;
	}

	if(gears_renderer_createCacheAndPools(self) == 0)
	{
		goto fail_cacheAndPools;
	}

	if(gears_renderer_createSwapchain(self) == 0)
	{
		goto fail_swapchain;
	}

	if(gears_renderer_createRenderpass(self) == 0)
	{
		goto fail_renderpass;
	}

	if(gears_renderer_createDepth(self) == 0)
	{
		goto fail_depth;
	}

	if(gears_renderer_createFramebuffer(self) == 0)
	{
		goto fail_framebuffer;
	}

	if(gears_renderer_createCommandBuffers(self) == 0)
	{
		goto fail_command_buffers;
	}

	if(gears_renderer_createSemaphores(self) == 0)
	{
		goto fail_semaphores;
	}

	if(gears_renderer_createModules(self) == 0)
	{
		goto fail_modules;
	}

	if(gears_renderer_createDescriptorSetLayout(self) == 0)
	{
		goto fail_dsl;
	}

	if(gears_renderer_createPipelineLayout(self) == 0)
	{
		goto fail_pl;
	}

	if(gears_renderer_createGraphicsPipeline(self) == 0)
	{
		goto fail_gp;
	}

	self->view_scale = 1.0f;

	a3d_quaternion_t qx;
	a3d_quaternion_t qy;
	a3d_quaternion_loadeuler(&qx, 20.0f,  0.0f, 0.0f);
	a3d_quaternion_loadeuler(&qy,  0.0f, 30.0f, 0.0f);
	a3d_quaternion_rotateq_copy(&qy, &qx, &self->view_q);

	self->angle = 0.0f;

	a3d_mat4f_identity(&self->pm);
	a3d_mat4f_identity(&self->mvm);

	self->t0       = a3d_timestamp();
	self->t_last   = 0.0;
	self->frames   = 0;
	self->last_fps = 0.0f;

	// create the gears buffers
	self->gear1 = gear_new(self, &RED,
	                       1.0f, 4.0f, 1.0f, 20, 0.7f);
	if(self->gear1 == NULL)
	{
		goto fail_gear1;
	}

	self->gear2 = gear_new(self, &GREEN,
	                       0.5f, 2.0f, 2.0f, 10, 0.7f);
	if(self->gear2 == NULL)
	{
		goto fail_gear2;
	}

	self->gear3 = gear_new(self, &BLUE,
	                       1.3f, 2.0f, 0.5f, 10, 0.7f);
	if(self->gear3 == NULL)
	{
		goto fail_gear3;
	}

	self->mvm_stack = a3d_stack4f_new();
	if(self->mvm_stack == NULL)
	{
		goto fail_stack;
	}

	self->cmd_fn = cmd_fn;

	// success
	return self;

	// failure
	fail_stack:
		gear_delete(&self->gear3);
	fail_gear3:
		gear_delete(&self->gear2);
	fail_gear2:
		gear_delete(&self->gear1);
	fail_gear1:
		vkDestroyPipeline(self->device,
		                  self->pipeline, NULL);
	fail_gp:
		vkDestroyPipelineLayout(self->device,
		                        self->pipeline_layout,
		                        NULL);
	fail_pl:
		vkDestroyDescriptorSetLayout(self->device,
		                             self->descriptor_set_layout,
		                             NULL);
	fail_dsl:
		vkDestroyShaderModule(self->device,
		                      self->module_frag, NULL);
		vkDestroyShaderModule(self->device,
		                      self->module_vert, NULL);
	fail_modules:
	{
		int i;
		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[i],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);
		}
		free(self->semaphore_submit);
		free(self->semaphore_acquire);
	}
	fail_semaphores:
		vkFreeCommandBuffers(self->device,
		                     self->command_pool,
		                     self->swapchain_image_count,
		                     self->command_buffers);
		free(self->command_buffers);
	fail_command_buffers:
		gears_renderer_destroyFramebuffer(self);
	fail_framebuffer:
		gears_renderer_destroyDepth(self);
	fail_depth:
		vkDestroyRenderPass(self->device,
		                    self->render_pass, NULL);
	fail_renderpass:
		gears_renderer_destroySwapchain(self);
	fail_swapchain:
		vkDestroyDescriptorPool(self->device,
		                        self->descriptor_pool,
		                        NULL);
		vkDestroyCommandPool(self->device,
		                     self->command_pool, NULL);
		vkDestroyPipelineCache(self->device,
		                       self->pipeline_cache, NULL);
	fail_cacheAndPools:
		vkDestroyDevice(self->device, NULL);
	fail_device:
	fail_physical_device:
		vkDestroySurfaceKHR(self->instance,
		                    self->surface, NULL);
	fail_surface:
		vkDestroyInstance(self->instance, NULL);
	fail_instance:
		#ifndef ANDROID
			SDL_DestroyWindow(self->window);
			SDL_Quit();
		#endif
		free(self);
	return NULL;
}

void gears_renderer_delete(gears_renderer_t** _self)
{
	// *_self can be null
	assert(_self);

	gears_renderer_t* self = *_self;
	if(self)
	{
		vkDeviceWaitIdle(self->device);

		gear_delete(&self->gear3);
		gear_delete(&self->gear2);
		gear_delete(&self->gear1);
		a3d_stack4f_delete(&self->mvm_stack);

		vkDestroyPipeline(self->device,
		                  self->pipeline, NULL);
		vkDestroyPipelineLayout(self->device,
		                        self->pipeline_layout,
		                        NULL);
		vkDestroyDescriptorSetLayout(self->device,
		                             self->descriptor_set_layout,
		                             NULL);
		vkDestroyShaderModule(self->device,
		                      self->module_frag, NULL);
		vkDestroyShaderModule(self->device,
		                      self->module_vert, NULL);

		int i;
		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[i],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);
		}
		free(self->semaphore_submit);
		free(self->semaphore_acquire);

		vkFreeCommandBuffers(self->device,
		                     self->command_pool,
		                     self->swapchain_image_count,
		                     self->command_buffers);
		free(self->command_buffers);

		gears_renderer_destroyFramebuffer(self);

		gears_renderer_destroyDepth(self);

		vkDestroyRenderPass(self->device,
		                    self->render_pass, NULL);

		gears_renderer_destroySwapchain(self);

		vkDestroyDescriptorPool(self->device,
		                        self->descriptor_pool,
		                        NULL);
		vkDestroyCommandPool(self->device,
		                     self->command_pool, NULL);
		vkDestroyPipelineCache(self->device,
		                       self->pipeline_cache, NULL);
		vkDestroyDevice(self->device, NULL);
		vkDestroySurfaceKHR(self->instance,
		                    self->surface, NULL);
		vkDestroyInstance(self->instance, NULL);
		#ifndef ANDROID
			SDL_DestroyWindow(self->window);
			SDL_Quit();
		#endif
		free(self);
		*_self = NULL;
	}
}

void gears_renderer_draw(gears_renderer_t* self)
{
	assert(self);

	VkSemaphore semaphore_acquire;
	VkSemaphore semaphore_submit;
	gears_renderer_nextSemaphore(self,
	                             &semaphore_acquire,
	                             &semaphore_submit);

	uint64_t timeout = 250000000;
	if(vkAcquireNextImageKHR(self->device,
	                         self->swapchain,
	                         timeout,
	                         semaphore_acquire,
	                         VK_NULL_HANDLE,
	                         &self->swapchain_frame) != VK_SUCCESS)
	{
		// failure typically caused by resizes
		return;
	}

	VkFence swapchain_fence = self->swapchain_fences[self->swapchain_frame];
	vkWaitForFences(self->device, 1,
	                &swapchain_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(self->device, 1, &swapchain_fence);

	gears_renderer_step(self);

	VkCommandBuffer command_buffer;
	command_buffer = self->command_buffers[self->swapchain_frame];

	VkFramebuffer framebuffer;
	framebuffer = self->framebuffers[self->swapchain_frame];

	if(vkResetCommandBuffer(command_buffer, 0) != VK_SUCCESS)
	{
		LOGE("vkResetCommandBuffer failed");
		return;
	}

	VkCommandBufferInheritanceInfo cbi_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.pNext = NULL,
		.renderPass           = self->render_pass,
		.subpass              = 0,
		.framebuffer          = framebuffer,
		.occlusionQueryEnable = VK_FALSE,
		.queryFlags           = 0,
		.pipelineStatistics   = 0
	};

	VkCommandBufferBeginInfo cb_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = &cbi_info
	};

	if(vkBeginCommandBuffer(command_buffer,
	                        &cb_info) != VK_SUCCESS)
	{
		LOGE("vkBeginCommandBuffer failed");
		return;
	}

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
		.image               = self->swapchain_images[self->swapchain_frame],
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

	if(self->depth_transition)
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
			.image               = self->depth_image,
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

		self->depth_transition = 0;
	}

	VkViewport viewport =
	{
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float) self->swapchain_extent.width,
		.height   = (float) self->swapchain_extent.height,
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
			.width  = (uint32_t) self->swapchain_extent.width,
			.height = (uint32_t) self->swapchain_extent.height,
		}
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

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

	uint32_t width  = self->swapchain_extent.width;
	uint32_t height = self->swapchain_extent.height;
	VkRenderPassBeginInfo rp_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass      = self->render_pass,
		.framebuffer     = self->framebuffers[self->swapchain_frame],
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
	                  self->pipeline);

	gear_draw(self->gear1, command_buffer);
	gear_draw(self->gear2, command_buffer);
	gear_draw(self->gear3, command_buffer);

	vkCmdEndRenderPass(command_buffer);

	vkEndCommandBuffer(command_buffer);

	VkPipelineStageFlags wait_dst_stage_mask;
	wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo s_info =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount   = 1,
		.pWaitSemaphores      = &semaphore_acquire,
		.pWaitDstStageMask    = &wait_dst_stage_mask,
		.commandBufferCount   = 1,
		.pCommandBuffers      = &command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores    = &semaphore_submit
	};

	if(vkQueueSubmit(self->queue, 1, &s_info,
	                 swapchain_fence) != VK_SUCCESS)
	{
		LOGE("vkQueueSubmit failed");
		return;
	}

	VkPresentInfoKHR p_info =
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = &semaphore_submit,
		.swapchainCount     = 1,
		.pSwapchains        = &self->swapchain,
		.pImageIndices      = &self->swapchain_frame,
		.pResults           = NULL
	};

	if(vkQueuePresentKHR(self->queue,
	                     &p_info) != VK_SUCCESS)
	{
		// failure typically caused by resizes
		return;
	}
}

int gears_renderer_resize(gears_renderer_t* self)
{
	assert(self);

	vkDeviceWaitIdle(self->device);

	gears_renderer_destroyDepth(self);
	gears_renderer_destroyFramebuffer(self);
	gears_renderer_destroySwapchain(self);

	if(gears_renderer_createSwapchain(self) == 0)
	{
		return 0;
	}

	if(gears_renderer_createDepth(self) == 0)
	{
		goto fail_depth;
	}

	if(gears_renderer_createFramebuffer(self) == 0)
	{
		goto fail_framebuffer;
	}

	// success
	return 1;

	// failure
	fail_framebuffer:
		gears_renderer_destroyDepth(self);
	fail_depth:
		gears_renderer_destroySwapchain(self);
	return 0;
}

void gears_renderer_touch(gears_renderer_t* self,
                          int action, int count, double ts,
                          float x0, float y0,
                          float x1, float y1,
                          float x2, float y2,
                          float x3, float y3)
{
	assert(self);

	if(action == GEARS_TOUCH_ACTION_UP)
	{
		// Do nothing
		self->touch_state = GEARS_TOUCH_STATE_INIT;
	}
	else if(count == 1)
	{
		if(self->touch_state == GEARS_TOUCH_STATE_ROTATE)
		{
			float dx = x0 - self->touch_x1;
			float dy = y0 - self->touch_y1;
			gears_renderer_rotate(self, dx, dy);
			self->touch_x1 = x0;
			self->touch_y1 = y0;
		}
		else if(action == GEARS_TOUCH_ACTION_DOWN)
		{
			self->touch_x1    = x0;
			self->touch_y1    = y0;
			self->touch_state = GEARS_TOUCH_STATE_ROTATE;
		}
	}
	else if(count == 2)
	{
		if(self->touch_state == GEARS_TOUCH_STATE_ZOOM)
		{
			float dx = fabsf(x1 - x0);
			float dy = fabsf(y1 - y0);
			float ds = sqrtf(dx*dx + dy*dy);

			gears_renderer_scale(self, ds/self->touch_ds);

			self->touch_ds = ds;
		}
		else
		{
			float dx = fabsf(x1 - x0);
			float dy = fabsf(y1 - y0);
			float ds = sqrtf(dx*dx + dy*dy);

			self->touch_ds    = ds;
			self->touch_state = GEARS_TOUCH_STATE_ZOOM;
		}
	}
}

void gears_renderer_keyPress(gears_renderer_t* self,
                             int keycode, int meta)
{
	assert(self);

	if(keycode == A3D_KEY_ESCAPE)
	{
		// double tap back to exit
		double t1 = a3d_timestamp();
		if((t1 - self->escape_t0) < 0.5)
		{
			gears_renderer_exit(self);
		}
		else
		{
			self->escape_t0 = t1;
		}
	}
}

int
gears_renderer_getMemoryTypeIndex(gears_renderer_t* self,
                                  uint32_t mt_bits,
                                  VkFlags mp_flags,
                                  uint32_t* mt_index)
{
	assert(self);
	assert(mt_index);

	VkPhysicalDeviceMemoryProperties mp;
	vkGetPhysicalDeviceMemoryProperties(self->physical_device,
	                                    &mp);

	// find a memory type that meets the memory requirements
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

	LOGE("invalid memory type");
	return 0;
}
