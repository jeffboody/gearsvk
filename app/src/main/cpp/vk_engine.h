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

#ifndef vk_engine_h
#define vk_engine_h

#include "a3d_math.h"
#ifdef ANDROID
	#include <vulkan_wrapper.h>
	#include <android_native_app_glue.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_vulkan.h>
	#include <vulkan/vulkan.h>
#endif

/***********************************************************
* vk_engine_t                                              *
***********************************************************/

typedef struct
{
	#ifdef ANDROID
		struct android_app* app;
	#else
		SDL_Window* window;
	#endif

	VkInstance instance;

	// surface state
	VkSurfaceKHR surface;

	VkPhysicalDevice physical_device;

	// device state
	VkDevice device;
	uint32_t queue_family_index;
	VkQueue  queue;

	// cache and pool state (optimizers)
	VkPipelineCache  pipeline_cache;
	VkCommandPool    command_pool;
	VkDescriptorPool descriptor_pool;

	// swapchain state
	uint32_t        swapchain_frame;
	VkFormat        swapchain_format;
	VkExtent2D      swapchain_extent;
	VkColorSpaceKHR swapchain_color_space;
	uint32_t        swapchain_image_count;
	VkSwapchainKHR  swapchain;
	VkImage*        swapchain_images;

	// framebuffer state
	// one per swapchain image
	VkImageView*   framebuffer_image_views;
	VkFramebuffer* framebuffers;

	// render pass state
	VkRenderPass render_pass;

	// command buffers
	// one per swapchain image
	VkCommandBuffer* command_buffers;

	// synchronization
	// one per swapchain image (acquire and submit)
	uint32_t     sync_index;
	VkSemaphore* semaphore_acquire;
	VkSemaphore* semaphore_submit;
	VkFence*     fence_submit;

	// uniform data
	a3d_mat4f_t mvp;

	// shaders, descriptors, layout and pipeline
	VkShaderModule        module_vert;
	VkShaderModule        module_frag;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet       descriptor_set;
	VkPipelineLayout      pipeline_layout;
	VkPipeline            pipeline;

	// buffers
	VkBuffer       uniform_buffer;
	VkDeviceMemory uniform_memory;
	VkBuffer       vertex_buffer;
	VkDeviceMemory vertex_memory;
	VkBuffer       index_buffer;
	VkDeviceMemory index_memory;
} vk_engine_t;

vk_engine_t* vk_engine_new(void* app,
                           const char* app_name,
                           uint32_t app_version);
void         vk_engine_delete(vk_engine_t** _self);
void         vk_engine_nextSync(vk_engine_t* self,
                                VkSemaphore* semaphore_acquire,
                                VkSemaphore* semaphore_submit,
                                VkFence*     fence_submit);
void         vk_engine_draw(vk_engine_t* self);

#endif
