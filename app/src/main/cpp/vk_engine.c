/*
 * Copyright (c) 2019 Jeff Boody
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
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "vk_engine.h"

#define LOG_TAG "gearsvk"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static a3d_vec3f_t VERTICES[] =
{
	{ -0.5f,  0.5f, 0.0f },
	{ -0.5f, -0.5f, 0.0f },
	{  0.5f,  0.5f, 0.0f },
	{  0.5f, -0.5f, 0.0f },
};

static unsigned short INDICES[] =
{
	0, 1, 2, 3
};

static int
vk_engine_hasDeviceExtensions(vk_engine_t* self,
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
vk_engine_initSDL(vk_engine_t* self,
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
	            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
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
vk_engine_createInstance(vk_engine_t* self,
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
		.pEngineName        = "vk-engine",
		.engineVersion      = VK_MAKE_VERSION(1,0,0),
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
vk_engine_createSurface(vk_engine_t* self)
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
vk_engine_getPhysicalDevice(vk_engine_t* self)
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
vk_engine_createDevice(vk_engine_t* self)
{
	assert(self);

	uint32_t    extension_count   = 1;
	const char* extension_names[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	if(vk_engine_hasDeviceExtensions(self, extension_count,
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
	LOGI("Device Queue: queueFlags");
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

		LOGI("%i: queueFlags=0x%X", i, (int) qfp[i].queueFlags);
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
vk_engine_createCacheAndPools(vk_engine_t* self)
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
vk_engine_createSwapchain(vk_engine_t* self)
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

	if(vkGetSwapchainImagesKHR(self->device,
	                           self->swapchain,
	                           &self->swapchain_image_count,
	                           NULL) != VK_SUCCESS)
	{
		LOGE("vkGetSwapchainImagesKHR failed");
		goto fail_get1;
	}

	self->swapchain_images = (VkImage*)
	                         calloc(self->swapchain_image_count,
	                                sizeof(VkImage));
	if(self->swapchain_images == NULL)
	{
		LOGE("calloc failed");
		goto fail_images;
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

	free(pm);
	free(sf);

	// success
	return 1;

	// failure
	fail_get2:
		free(self->swapchain_images);
	fail_images:
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

static int
vk_engine_createFramebuffer(vk_engine_t* self)
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

		VkFramebufferCreateInfo f_info =
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.renderPass = self->render_pass,
			.attachmentCount = 1,
			.pAttachments    = &self->framebuffer_image_views[i],
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
		}
		free(self->framebuffers);
	}
	fail_alloc_framebuffers:
		free(self->framebuffer_image_views);
	return 0;
}

static int
vk_engine_createRenderpass(vk_engine_t* self)
{
	assert(self);

	VkAttachmentDescription attachment =
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
	};

	VkAttachmentReference color_attachment =
	{
		.attachment = 0,
		.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass =
	{
		.flags = 0,
		.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount     = 0,
		.pInputAttachments        = NULL,
		.colorAttachmentCount     = 1,
		.pColorAttachments        = &color_attachment,
		.pResolveAttachments      = NULL,
		.pDepthStencilAttachment  = NULL,
		.preserveAttachmentCount  = 0,
		.pPreserveAttachments     = NULL,
	};

	VkRenderPassCreateInfo rp_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = 1,
		.pAttachments    = &attachment,
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
vk_engine_createCommandBuffers(vk_engine_t* self)
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
vk_engine_createSync(vk_engine_t* self)
{
	assert(self);

	self->sync_index = 0;

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

	self->fence_submit = (VkFence*)
	                     calloc(self->swapchain_image_count,
	                            sizeof(VkFence));
	if(self->fence_submit == NULL)
	{
		LOGE("calloc failed");
		goto fail_alloc_fs;
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

		VkFenceCreateInfo f_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		if(vkCreateFence(self->device, &f_info, NULL,
		                 &self->fence_submit[i]) != VK_SUCCESS)
		{
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[i],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);
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
			vkDestroyFence(self->device,
			               self->fence_submit[j], NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[j],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[j],
			                   NULL);
		}
	}
	fail_alloc_fs:
		free(self->semaphore_submit);
	fail_alloc_ss:
		free(self->semaphore_acquire);
	return 0;
}

static uint32_t*
vk_engine_importModule(vk_engine_t* self,
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
vk_engine_getMemoryTypeIndex(vk_engine_t* self,
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

static int
vk_engine_createUniformBuffer(vk_engine_t* self)
{
	assert(self);

	// layout(set=0, binding=0) uniform uniformBuffer
	// {
	//     mat4 mvp;
	// };
	VkBufferCreateInfo b_info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size  = sizeof(a3d_mat4f_t),
		.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &self->queue_family_index
	};

	if(vkCreateBuffer(self->device, &b_info, NULL,
	                  &self->uniform_buffer) != VK_SUCCESS)
	{
		LOGE("vkCreateBuffer failed");
		return 0;
	}

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(self->device,
	                              self->uniform_buffer,
	                              &mr);

	VkFlags mp_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t mt_index;
	if(vk_engine_getMemoryTypeIndex(self,
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
	                    &self->uniform_memory) != VK_SUCCESS)
	{
		LOGE("vkAllocateMemory failed");
		goto fail_allocate;
	}

	void* data;
	if(vkMapMemory(self->device, self->uniform_memory,
	               0, mr.size, 0, &data) != VK_SUCCESS)
	{
		LOGE("vkMapMemory failed");
		goto fail_map;
	}
	memcpy(data, (const void*) &self->mvp,
	       sizeof(a3d_mat4f_t));
	vkUnmapMemory(self->device, self->uniform_memory);

	if(vkBindBufferMemory(self->device,
	                      self->uniform_buffer,
	                      self->uniform_memory,
	                      0) != VK_SUCCESS)
	{
		LOGE("vkBindBufferMemory failed");
		goto fail_bind;
	}

	// success
	return 1;

	// failure
	fail_bind:
	fail_map:
		vkFreeMemory(self->device,
		             self->uniform_memory,
		             NULL);
	fail_allocate:
	fail_memory_type:
		vkDestroyBuffer(self->device,
		                self->uniform_buffer,
		                NULL);
	return 0;
}

static int
vk_engine_createVertexBuffer(vk_engine_t* self)
{
	assert(self);

	VkBufferCreateInfo b_info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size  = sizeof(VERTICES),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &self->queue_family_index
	};

	if(vkCreateBuffer(self->device, &b_info, NULL,
	                  &self->vertex_buffer) != VK_SUCCESS)
	{
		LOGE("vkCreateBuffer failed");
		return 0;
	}

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(self->device,
	                              self->vertex_buffer,
	                              &mr);

	VkFlags mp_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t mt_index;
	if(vk_engine_getMemoryTypeIndex(self,
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
	                    &self->vertex_memory) != VK_SUCCESS)
	{
		LOGE("vkAllocateMemory failed");
		goto fail_allocate;
	}

	void* data;
	if(vkMapMemory(self->device, self->vertex_memory,
	               0, mr.size, 0, &data) != VK_SUCCESS)
	{
		LOGE("vkMapMemory failed");
		goto fail_map;
	}
	memcpy(data, (const void*) VERTICES, sizeof(VERTICES));
	vkUnmapMemory(self->device, self->vertex_memory);

	if(vkBindBufferMemory(self->device,
	                      self->vertex_buffer,
	                      self->vertex_memory,
	                      0) != VK_SUCCESS)
	{
		LOGE("vkBindBufferMemory failed");
		goto fail_bind;
	}

	// success
	return 1;

	// failure
	fail_bind:
	fail_map:
		vkFreeMemory(self->device,
		             self->vertex_memory,
		             NULL);
	fail_allocate:
	fail_memory_type:
		vkDestroyBuffer(self->device,
		                self->vertex_buffer,
		                NULL);
	return 0;
}

static int
vk_engine_createIndexBuffer(vk_engine_t* self)
{
	assert(self);

	VkBufferCreateInfo b_info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size  = sizeof(INDICES),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &self->queue_family_index
	};

	if(vkCreateBuffer(self->device, &b_info, NULL,
	                  &self->index_buffer) != VK_SUCCESS)
	{
		LOGE("vkCreateBuffer failed");
		return 0;
	}

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(self->device,
	                              self->index_buffer,
	                              &mr);

	VkFlags mp_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t mt_index;
	if(vk_engine_getMemoryTypeIndex(self,
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
	                    &self->index_memory) != VK_SUCCESS)
	{
		LOGE("vkAllocateMemory failed");
		goto fail_allocate;
	}

	void* data;
	if(vkMapMemory(self->device, self->index_memory,
	               0, mr.size, 0, &data) != VK_SUCCESS)
	{
		LOGE("vkMapMemory failed");
		goto fail_map;
	}
	memcpy(data, (const void*) INDICES, sizeof(INDICES));
	vkUnmapMemory(self->device, self->index_memory);

	if(vkBindBufferMemory(self->device,
	                      self->index_buffer,
	                      self->index_memory,
	                      0) != VK_SUCCESS)
	{
		LOGE("vkBindBufferMemory failed");
		goto fail_bind;
	}

	// success
	return 1;

	// failure
	fail_bind:
	fail_map:
		vkFreeMemory(self->device,
		             self->index_memory,
		             NULL);
	fail_allocate:
	fail_memory_type:
		vkDestroyBuffer(self->device,
		                self->index_buffer,
		                NULL);
	return 0;
}

static int
vk_engine_createModules(vk_engine_t* self)
{
	assert(self);

	size_t    size_vert = 0;
	uint32_t* code_vert;
	code_vert = vk_engine_importModule(self,
	                                   "shaders/vert.spv",
	                                   &size_vert);
	if(code_vert == NULL)
	{
		return 0;
	}

	size_t    size_frag = 0;
	uint32_t* code_frag;
	code_frag = vk_engine_importModule(self,
	                                   "shaders/frag.spv",
	                                   &size_frag);
	if(code_frag == NULL)
	{
		goto fail_import_frag;
	}

	VkShaderModuleCreateInfo vsm_info =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = size_vert,
		.pCode    = code_vert
	};

	if(vkCreateShaderModule(self->device, &vsm_info, NULL,
	                        &self->module_vert) != VK_SUCCESS)
	{
		LOGE("vkCreateShaderModule failed");
		goto fail_vert;
	}

	VkShaderModuleCreateInfo fsm_info =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = size_frag,
		.pCode    = code_frag
	};

	if(vkCreateShaderModule(self->device, &fsm_info, NULL,
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
vk_engine_createDescriptorSetLayout(vk_engine_t* self)
{
	assert(self);

	VkDescriptorSetLayoutBinding binding_uniformBuffer =
	{
		.binding = 0,
		.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = NULL
	};

	VkDescriptorSetLayoutCreateInfo dsl_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = 1,
		.pBindings    = &binding_uniformBuffer,
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
vk_engine_createDescriptorSet(vk_engine_t* self)
{
	assert(self);

	VkDescriptorSetAllocateInfo ds_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool     = self->descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &self->descriptor_set_layout
	};

	if(vkAllocateDescriptorSets(self->device, &ds_info,
	                            &self->descriptor_set) != VK_SUCCESS)
	{
		LOGE("vkAllocateDescriptorSets failed");
		return 0;
	}

	// layout(set=0, binding=0) uniform uniformBuffer
	// {
	//     mat4 mvp;
	// };
	VkDescriptorBufferInfo db_info =
	{
		.buffer  = self->uniform_buffer,
		.offset  = 0,
		.range   = sizeof(a3d_mat4f_t)
	};

	VkWriteDescriptorSet writes =
	{
		.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext  = NULL,
		.dstSet = self->descriptor_set,
		.dstBinding       = 0,
		.dstArrayElement  = 0,
		.descriptorCount  = 1,
		.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pImageInfo       = NULL,
		.pBufferInfo      = &db_info,
		.pTexelBufferView = NULL,
	};

	vkUpdateDescriptorSets(self->device, 1, &writes,
	                       0, NULL);

	return 1;
}

static int
vk_engine_createPipelineLayout(vk_engine_t* self)
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
vk_engine_createGraphicsPipeline(vk_engine_t* self)
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
	VkVertexInputBindingDescription vib =
	{
		.binding   = 0,
		.stride    = (uint32_t) sizeof(a3d_vec3f_t),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	// layout(location=0) in vec3 vertex;
	VkVertexInputAttributeDescription via =
	{
		.location = 0,
		.binding  = 0,
		.format   = VK_FORMAT_R32G32B32_SFLOAT,
		.offset   = 0
	};

	VkPipelineVertexInputStateCreateInfo pvis_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount   = 1,
		.pVertexBindingDescriptions      = &vib,
		.vertexAttributeDescriptionCount = 1,
		.pVertexAttributeDescriptions    = &via
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
		.depthTestEnable        = VK_FALSE,
		.depthWriteEnable       = VK_FALSE,
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

	VkPipelineDynamicStateCreateInfo pds_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = 0,
		.pDynamicStates    = NULL,
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

/***********************************************************
* public                                                   *
***********************************************************/

vk_engine_t* vk_engine_new(void* app,
                           const char* app_name,
                           uint32_t app_version)
{
	#ifdef ANDROID
		assert(app);
	#else
		assert(app == NULL);
	#endif
	assert(app_name);

	vk_engine_t* self;
	self = (vk_engine_t*)
	       calloc(1, sizeof(vk_engine_t));
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	#ifdef ANDROID
		self->app = (struct android_app*) app;
	#else
		if(vk_engine_initSDL(self, app_name) == 0)
		{
			free(self);
			return NULL;
		}
	#endif

	if(vk_engine_createInstance(self, app_name,
	                            app_version) == 0)
	{
		goto fail_instance;
	}

	if(vk_engine_createSurface(self) == 0)
	{
		goto fail_surface;
	}

	if(vk_engine_getPhysicalDevice(self) == 0)
	{
		goto fail_physical_device;
	}

	if(vk_engine_createDevice(self) == 0)
	{
		goto fail_device;
	}

	if(vk_engine_createCacheAndPools(self) == 0)
	{
		goto fail_cacheAndPools;
	}

	if(vk_engine_createSwapchain(self) == 0)
	{
		goto fail_swapchain;
	}

	if(vk_engine_createRenderpass(self) == 0)
	{
		goto fail_renderpass;
	}

	if(vk_engine_createFramebuffer(self) == 0)
	{
		goto fail_framebuffer;
	}

	if(vk_engine_createCommandBuffers(self) == 0)
	{
		goto fail_command_buffers;
	}

	if(vk_engine_createSync(self) == 0)
	{
		goto fail_sync;
	}

	a3d_mat4f_ortho(&self->mvp, 1, -1.0f, 1.0f,
	                -1.0f, 1.0f, 0.0f, 2.0f);
	a3d_mat4f_translate(&self->mvp,
	                    0, 0.0f, 0.0f, -1.0f);

	if(vk_engine_createUniformBuffer(self) == 0)
	{
		goto fail_uniform_buffer;
	}

	if(vk_engine_createVertexBuffer(self) == 0)
	{
		goto fail_vertex_buffer;
	}

	if(vk_engine_createIndexBuffer(self) == 0)
	{
		goto fail_index_buffer;
	}

	if(vk_engine_createModules(self) == 0)
	{
		goto fail_modules;
	}

	if(vk_engine_createDescriptorSetLayout(self) == 0)
	{
		goto fail_dsl;
	}

	if(vk_engine_createDescriptorSet(self) == 0)
	{
		goto fail_ds;
	}

	if(vk_engine_createPipelineLayout(self) == 0)
	{
		goto fail_pl;
	}

	if(vk_engine_createGraphicsPipeline(self) == 0)
	{
		goto fail_gp;
	}

	// success
	return self;

	// failure
	fail_gp:
		vkDestroyPipelineLayout(self->device,
		                        self->pipeline_layout,
		                        NULL);
	fail_pl:
	fail_ds:
		vkDestroyDescriptorSetLayout(self->device,
		                             self->descriptor_set_layout,
		                             NULL);
	fail_dsl:
		vkDestroyShaderModule(self->device,
		                      self->module_frag, NULL);
		vkDestroyShaderModule(self->device,
		                      self->module_vert, NULL);
	fail_modules:
		vkFreeMemory(self->device,
		             self->index_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->index_buffer,
		                NULL);
	fail_index_buffer:
		vkFreeMemory(self->device,
		             self->vertex_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->vertex_buffer,
		                NULL);
	fail_vertex_buffer:
		vkFreeMemory(self->device,
		             self->uniform_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->uniform_buffer,
		                NULL);
	fail_uniform_buffer:
	{
		int i;
		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroyFence(self->device,
			               self->fence_submit[i], NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[i],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);
		}
	}
	fail_sync:
		vkFreeCommandBuffers(self->device,
		                     self->command_pool,
		                     self->swapchain_image_count,
		                     self->command_buffers);
		free(self->command_buffers);
	fail_command_buffers:
	{
		int i;
		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroyFramebuffer(self->device,
			                     self->framebuffers[i],
			                     NULL);
			vkDestroyImageView(self->device,
			                   self->framebuffer_image_views[i],
			                   NULL);
		}
		free(self->framebuffers);
		free(self->framebuffer_image_views);
	}
	fail_framebuffer:
		vkDestroyRenderPass(self->device,
		                    self->render_pass, NULL);
	fail_renderpass:
		free(self->swapchain_images);
		vkDestroySwapchainKHR(self->device,
		                      self->swapchain, NULL);
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

void vk_engine_delete(vk_engine_t** _self)
{
	assert(_self);

	vk_engine_t* self = *_self;
	if(self)
	{
		vkQueueWaitIdle(self->queue);
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
		vkFreeMemory(self->device,
		             self->index_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->index_buffer,
		                NULL);
		vkFreeMemory(self->device,
		             self->vertex_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->vertex_buffer,
		                NULL);
		vkFreeMemory(self->device,
		             self->uniform_memory,
		             NULL);
		vkDestroyBuffer(self->device,
		                self->uniform_buffer,
		                NULL);

		int i;
		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroyFence(self->device,
			               self->fence_submit[i], NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_submit[i],
			                   NULL);
			vkDestroySemaphore(self->device,
			                   self->semaphore_acquire[i],
			                   NULL);
		}

		vkFreeCommandBuffers(self->device,
		                     self->command_pool,
		                     self->swapchain_image_count,
		                     self->command_buffers);
		free(self->command_buffers);

		for(i = 0; i < self->swapchain_image_count; ++i)
		{
			vkDestroyFramebuffer(self->device,
			                     self->framebuffers[i],
			                     NULL);
			vkDestroyImageView(self->device,
			                   self->framebuffer_image_views[i],
			                   NULL);
		}
		free(self->framebuffers);
		free(self->framebuffer_image_views);

		vkDestroyRenderPass(self->device,
		                    self->render_pass, NULL);

		free(self->swapchain_images);
		vkDestroySwapchainKHR(self->device,
		                      self->swapchain, NULL);
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

void vk_engine_nextSync(vk_engine_t* self,
                        VkSemaphore* semaphore_acquire,
                        VkSemaphore* semaphore_submit,
                        VkFence*     fence_submit)
{
	assert(self);
	assert(semaphore_acquire);
	assert(semaphore_submit);
	assert(fence_submit);

	uint32_t idx       = self->sync_index;
	*semaphore_acquire = self->semaphore_acquire[idx];
	*semaphore_submit  = self->semaphore_submit[idx];
	*fence_submit      = self->fence_submit[idx];

	++idx;
	self->sync_index = idx%self->swapchain_image_count;
}

void vk_engine_draw(vk_engine_t* self)
{
	assert(self);

	VkSemaphore semaphore_acquire;
	VkSemaphore semaphore_submit;
	VkFence     fence_submit;
	vk_engine_nextSync(self,
	                   &semaphore_acquire,
	                   &semaphore_submit,
	                   &fence_submit);
	vkWaitForFences(self->device, 1,
	                &fence_submit, VK_TRUE, UINT64_MAX);
	vkResetFences(self->device, 1, &fence_submit);

	if(vkAcquireNextImageKHR(self->device,
	                         self->swapchain,
	                         UINT64_MAX,
	                         semaphore_acquire,
	                         VK_NULL_HANDLE,
	                         &self->swapchain_frame) != VK_SUCCESS)
	{
		LOGE("vkAcquireNextImageKHR failed");
		return;
	}

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

	VkClearValue cv =
	{
		.color        = { .float32={ 0.0f, 1.0f,
		                             0.0f, 1.0f } },
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
		.clearValueCount = 1,
		.pClearValues    = &cv
	};

	vkCmdBeginRenderPass(command_buffer,
	                     &rp_info,
	                     VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer,
	                  VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  self->pipeline);

	vkCmdBindDescriptorSets(command_buffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        self->pipeline_layout,
	                        0, 1, &self->descriptor_set,
	                        0, NULL);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(command_buffer, 0, 1,
	                       &self->vertex_buffer,
	                       &offset);
	vkCmdBindIndexBuffer(command_buffer,
	                     self->index_buffer,
	                     0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(command_buffer, 4, 1, 0, 0, 0);

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
	                 fence_submit) != VK_SUCCESS)
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
		LOGE("vkQueuePresentKHR failed");
		return;
	}
}
