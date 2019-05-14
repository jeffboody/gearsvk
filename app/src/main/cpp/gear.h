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

#ifndef gear_H
#define gear_H

#ifdef ANDROID
	#include <vulkan_wrapper.h>
	#include <android_native_app_glue.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_vulkan.h>
	#include <vulkan/vulkan.h>
#endif

#include "a3d/a3d_GL.h"
#include "a3d/math/a3d_mat4f.h"
#include "a3d/math/a3d_vec4f.h"

struct gears_renderer_s;

typedef struct
{
	struct gears_renderer_s* renderer;

	// color
	a3d_vec4f_t color;

	// dynamic buffer data
	// one per swapchain image
	VkBuffer*       uniformMvp_buffer;
	VkDeviceMemory* uniformMvp_memory;
	VkBuffer*       uniformNm_buffer;
	VkDeviceMemory* uniformNm_memory;

	// constant buffer data
	VkBuffer       uniformColor_buffer;
	VkDeviceMemory uniformColor_memory;

	// vertex data
	uint32_t       frontface_vc;
	VkBuffer       frontface_vertex_buffer;
	VkDeviceMemory frontface_vertex_memory;
	VkBuffer       frontface_normal_buffer;
	VkDeviceMemory frontface_normal_memory;
	uint32_t       backface_vc;
	VkBuffer       backface_vertex_buffer;
	VkDeviceMemory backface_vertex_memory;
	VkBuffer       backface_normal_buffer;
	VkDeviceMemory backface_normal_memory;
	uint32_t       outward_vc;
	VkBuffer       outward_vertex_buffer;
	VkDeviceMemory outward_vertex_memory;
	VkBuffer       outward_normal_buffer;
	VkDeviceMemory outward_normal_memory;
	uint32_t       cylinder_vc;
	VkBuffer       cylinder_vertex_buffer;
	VkDeviceMemory cylinder_vertex_memory;
	VkBuffer       cylinder_normal_buffer;
	VkDeviceMemory cylinder_normal_memory;

	// descriptors
	// one per swapchain image
	VkDescriptorSet* descriptor_sets;
} gear_t;

gear_t* gear_new(struct gears_renderer_s* renderer,
                 const a3d_vec4f_t* color,
                 float inner_radius, float outer_radius,
                 float width,
                 int teeth, float tooth_depth);
void    gear_delete(gear_t** _self);
void    gear_update(gear_t* self,
                    a3d_mat4f_t* mvp, a3d_mat4f_t* mvm);
void    gear_draw(gear_t* self,
                  VkCommandBuffer command_buffer);

#endif
