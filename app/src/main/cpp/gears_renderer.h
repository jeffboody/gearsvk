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

#ifndef gears_renderer_H
#define gears_renderer_H

#ifdef ANDROID
	#include <vulkan_wrapper.h>
	#include <android_native_app_glue.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_vulkan.h>
	#include <vulkan/vulkan.h>
#endif
#include "a3d/math/a3d_quaternion.h"
#include "a3d/math/a3d_mat4f.h"
#include "a3d/math/a3d_stack4f.h"
#include "gear.h"

/***********************************************************
* public                                                   *
***********************************************************/

typedef struct gears_renderer_s
{
	// window state
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
	VkFence*        swapchain_fences;

	// depth buffer
	int            depth_transition;
	VkImage        depth_image;
	VkDeviceMemory depth_memory;
	VkImageView    depth_image_view;

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
	// one per swapchain image
	uint32_t     semaphore_index;
	VkSemaphore* semaphore_acquire;
	VkSemaphore* semaphore_submit;

	// shaders, descriptors, layout and pipeline
	VkShaderModule        module_vert;
	VkShaderModule        module_frag;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout      pipeline_layout;
	VkPipeline            pipeline;

	// view state
	GLfloat          view_scale;
	a3d_quaternion_t view_q;

	// animation state
	GLfloat angle;

	// matrices
	a3d_mat4f_t    pm;
	a3d_mat4f_t    mvm;
	a3d_stack4f_t* mvm_stack;

	// fps state
	double t0;
	double t_last;
	int    frames;
	float  last_fps;

	// per-gear state
	gear_t* gear1;
	gear_t* gear2;
	gear_t* gear3;
} gears_renderer_t;

gears_renderer_t* gears_renderer_new(void* app,
                                     const char* app_name,
                                     uint32_t app_version);
void              gears_renderer_delete(gears_renderer_t** _self);
void              gears_renderer_draw(gears_renderer_t* self);
int               gears_renderer_getMemoryTypeIndex(gears_renderer_t* self,
                                                    uint32_t mt_bits,
                                                    VkFlags mp_flags,
                                                    uint32_t* mt_index);

#endif
