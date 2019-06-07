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
#include <math.h>
#include <string.h>
#include "gear.h"
#include "gears_renderer.h"
#include "a3d/a3d_glsm.h"
#include "a3d/a3d_shader.h"
#include "a3d/math/a3d_vec3f.h"
#include "a3d/math/a3d_vec4f.h"

#define LOG_TAG "gears"
#include "a3d/a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int
gear_createBuffer(gear_t* self,
                  size_t size, const void* buf,
                  VkBufferUsageFlags usage,
                  VkBuffer* _buffer,
                  VkDeviceMemory* _memory)
{
	// buf may be NULL
	assert(self);
	assert(_buffer);
	assert(_memory);

	gears_renderer_t* renderer = self->renderer;

	VkBufferCreateInfo b_info =
	{
		.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext                 = NULL,
		.flags                 = 0,
		.size                  = size,
		.usage                 = usage,
		.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices   = &renderer->queue_family_index
	};

	if(vkCreateBuffer(renderer->device, &b_info, NULL,
	                  _buffer) != VK_SUCCESS)
	{
		LOGE("vkCreateBuffer failed");
		return 0;
	}

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(renderer->device,
	                              *_buffer,
	                              &mr);

	VkFlags mp_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t mt_index;
	if(gears_renderer_getMemoryTypeIndex(renderer,
	                                     mr.memoryTypeBits,
	                                     mp_flags,
	                                     &mt_index) == 0)
	{
		goto fail_memory_type;
	}

	VkMemoryAllocateInfo ma_info =
	{
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext           = NULL,
		.allocationSize  = mr.size,
		.memoryTypeIndex = mt_index
	};

	if(vkAllocateMemory(renderer->device, &ma_info, NULL,
	                    _memory) != VK_SUCCESS)
	{
		LOGE("vkAllocateMemory failed");
		goto fail_allocate;
	}

	if(buf)
	{
		void* data;
		if(vkMapMemory(renderer->device, *_memory,
		               0, mr.size, 0, &data) != VK_SUCCESS)
		{
			LOGE("vkMapMemory failed");
			goto fail_map;
		}
		memcpy(data, buf, size);
		vkUnmapMemory(renderer->device, *_memory);
	}

	if(vkBindBufferMemory(renderer->device, *_buffer,
	                      *_memory, 0) != VK_SUCCESS)
	{
		LOGE("vkBindBufferMemory failed");
		goto fail_bind;
	}

	// success
	return 1;

	// failure
	fail_bind:
	fail_map:
		vkFreeMemory(renderer->device,
		             *_memory,
		             NULL);
	fail_allocate:
	fail_memory_type:
		vkDestroyBuffer(renderer->device,
		                *_buffer,
		                NULL);
	return 0;
}

static void gear_angle(int i, int teeth, float* a0, float* a1, float* a2, float* a3)
{
	/*
	 *              +-----+      r2
	 *             /       \
	 *            /         \
	 *           /           \
	 *          /             \
	 *   +-----+               + r1
	 *
	 *         a3   a2   a1    a0
	 *
	 *   +-----+---------------+ r0
	 */

	float angle = i * 2.0f * M_PI / teeth;
	float da = 2.0f * M_PI / teeth / 4.0f;
	*a0 = angle;
	*a1 = angle + 1.0f * da;
	*a2 = angle + 2.0f * da;
	*a3 = angle + 3.0f * da;
}

/***********************************************************
*  Generate a gear wheel.                                  *
*                                                          *
*  Input:  inner_radius - radius of hole at center         *
*          outer_radius - radius at center of teeth        *
*          width - width of gear                           *
*          teeth - number of teeth                         *
*          tooth_depth - depth of tooth                    *
***********************************************************/
static int gear_generate(gear_t* self,
                         float inner_radius, float outer_radius, float width,
                         int teeth, float tooth_depth)
{
	assert(self);

	gears_renderer_t* renderer = self->renderer;

	a3d_glsm_t* glsm = a3d_glsm_new();
	if(glsm == NULL)
	{
		return 0;
	}

	int i;
	float r0, r1, r2, dz;
	float a0, a1, a2, a3;
	float u, v, len;

	r0 = inner_radius;
	r1 = outer_radius - tooth_depth / 2.0f;
	r2 = outer_radius + tooth_depth / 2.0f;
	dz = 0.5f * width;

	// generate front face
	// GL_TRIANGLE_STRIP
	a3d_glsm_begin(glsm);
	a3d_glsm_normal3f(glsm, 0.0f, 0.0f, 1.0f);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
	}
	a3d_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), dz);
	a3d_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), dz);
	a3d_glsm_end(glsm);

	// buffer front face
	if((a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE) ||
	   (gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                      (const void*) glsm->vb,
	                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                      &self->frontface_vertex_buffer,
	                      &self->frontface_vertex_memory) == 0))
	{
		goto fail_createFrontfaceV;
	}
	self->frontface_vc = glsm->ec;

	if(gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                     (const void*) glsm->nb,
	                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                     &self->frontface_normal_buffer,
	                     &self->frontface_normal_memory) == 0)
	{
		goto fail_createFrontfaceN;
	}

	// generate back face
	// GL_TRIANGLE_STRIP
	a3d_glsm_begin(glsm);
	a3d_glsm_normal3f(glsm, 0.0f, 0.0f, -1.0f);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
	}
	a3d_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), -dz);
	a3d_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), -dz);
	a3d_glsm_end(glsm);

	// buffer back face
	if((a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE) ||
	   (gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                      (const void*) glsm->vb,
	                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                      &self->backface_vertex_buffer,
	                      &self->backface_vertex_memory) == 0))
	{
		goto fail_createBackfaceV;
	}
	self->backface_vc = glsm->ec;

	if(gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                     (const void*) glsm->nb,
	                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                     &self->backface_normal_buffer,
	                     &self->backface_normal_memory) == 0)
	{
		goto fail_createBackfaceN;
	}

	// generate outward faces of teeth
	// GL_TRIANGLE_STRIP
	// repeated vertices are necessary to achieve flat shading in ES2
	a3d_glsm_begin(glsm);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		if(i > 0)
		{
			a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
			a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		}

		u = r2 * cosf(a1) - r1 * cosf(a0);
		v = r2 * sinf(a1) - r1 * sinf(a0);
		len = sqrtf(u * u + v * v);
		u /= len;
		v /= len;

		a3d_glsm_normal3f(glsm, v, -u, 0.0f);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);

		a3d_glsm_normal3f(glsm, cosf(a0), sinf(a0), 0.0f);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);

		u = r1 * cosf(a3) - r2 * cosf(a2);
		v = r1 * sinf(a3) - r2 * sinf(a2);

		a3d_glsm_normal3f(glsm, v, -u, 0.0f);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		a3d_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);

		a3d_glsm_normal3f(glsm, cosf(a0), sinf(a0), 0.0f);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
		a3d_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);
	}
	a3d_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), dz);
	a3d_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), -dz);
	a3d_glsm_end(glsm);

	// buffer outward faces of teeth
	if((a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE) ||
	   (gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                      (const void*) glsm->vb,
	                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                      &self->outward_vertex_buffer,
	                      &self->outward_vertex_memory) == 0))
	{
		goto fail_createOutwardV;
	}
	self->outward_vc = glsm->ec;

	if(gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                     (const void*) glsm->nb,
	                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                     &self->outward_normal_buffer,
	                     &self->outward_normal_memory) == 0)
	{
		goto fail_createOutwardN;
	}

	// generate inside radius cylinder
	// GL_TRIANGLE_STRIP
	a3d_glsm_begin(glsm);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		a3d_glsm_normal3f(glsm, -cosf(a0), -sinf(a0), 0.0f);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		a3d_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
	}
	a3d_glsm_normal3f(glsm, -cosf(0.0f), -sinf(0.0f), 0.0f);
	a3d_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), -dz);
	a3d_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), dz);
	a3d_glsm_end(glsm);

	// buffer inside radius cylinder
	if((a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE) ||
	   (gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                      (const void*) glsm->vb,
	                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                      &self->cylinder_vertex_buffer,
	                      &self->cylinder_vertex_memory) == 0))
	{
		goto fail_createCylinderV;
	}
	self->cylinder_vc = glsm->ec;

	if(gear_createBuffer(self, glsm->ec*sizeof(a3d_vec3f_t),
	                     (const void*) glsm->nb,
	                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                     &self->cylinder_normal_buffer,
	                     &self->cylinder_normal_memory) == 0)
	{
		goto fail_createCylinderN;
	}

	// success
	a3d_glsm_delete(&glsm);
	return 1;

	// failure
	fail_createCylinderN:
		vkFreeMemory(renderer->device,
		             self->cylinder_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->cylinder_vertex_buffer,
		                NULL);
	fail_createCylinderV:
		vkFreeMemory(renderer->device,
		             self->outward_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_normal_buffer,
		                NULL);
	fail_createOutwardN:
		vkFreeMemory(renderer->device,
		             self->outward_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_vertex_buffer,
		                NULL);
	fail_createOutwardV:
		vkFreeMemory(renderer->device,
		             self->backface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_normal_buffer,
		                NULL);
	fail_createBackfaceN:
		vkFreeMemory(renderer->device,
		             self->backface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_vertex_buffer,
		                NULL);
	fail_createBackfaceV:
		vkFreeMemory(renderer->device,
		             self->frontface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_normal_buffer,
		                NULL);
	fail_createFrontfaceN:
		vkFreeMemory(renderer->device,
		             self->frontface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_vertex_buffer,
		                NULL);
	fail_createFrontfaceV:
		a3d_glsm_delete(&glsm);
	return 0;
}

static int
gear_createUniformMvpArray(gear_t* self)
{
	assert(self);

	gears_renderer_t* renderer;
	uint32_t          count;

	renderer = self->renderer;
	count    = renderer->swapchain_image_count;

	self->uniformMvp_buffer = (VkBuffer*)
	                          calloc(count,
	                                 sizeof(VkBuffer));
	if(self->uniformMvp_buffer == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	self->uniformMvp_memory = (VkDeviceMemory*)
	                          calloc(count,
	                                 sizeof(VkDeviceMemory));
	if(self->uniformMvp_memory == NULL)
	{
		LOGE("calloc failed");
		goto fail_memory;
	}

	int i;
	for(i = 0; i < count; ++i)
	{
		if(gear_createBuffer(self,
		                     sizeof(a3d_mat4f_t),
		                     NULL,
		                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                     &self->uniformMvp_buffer[i],
		                     &self->uniformMvp_memory[i]) == 0)
		{
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
			vkFreeMemory(renderer->device,
			             self->uniformMvp_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformMvp_buffer[i],
			                NULL);
		}
		free(self->uniformMvp_memory);
	}
	fail_memory:
		free(self->uniformMvp_buffer);
	return 0;
}

static int
gear_createUniformNmArray(gear_t* self)
{
	assert(self);

	gears_renderer_t* renderer;
	uint32_t          count;

	renderer = self->renderer;
	count    = renderer->swapchain_image_count;

	self->uniformNm_buffer = (VkBuffer*)
	                          calloc(count,
	                                 sizeof(VkBuffer));
	if(self->uniformNm_buffer == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	self->uniformNm_memory = (VkDeviceMemory*)
	                         calloc(count,
	                                sizeof(VkDeviceMemory));
	if(self->uniformNm_memory == NULL)
	{
		LOGE("calloc failed");
		goto fail_memory;
	}

	int i;
	for(i = 0; i < count; ++i)
	{
		if(gear_createBuffer(self,
		                     sizeof(a3d_mat4f_t),
		                     NULL,
		                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                     &self->uniformNm_buffer[i],
		                     &self->uniformNm_memory[i]) == 0)
		{
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
			vkFreeMemory(renderer->device,
			             self->uniformNm_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformNm_buffer[i],
			                NULL);
		}
		free(self->uniformNm_memory);
	}
	fail_memory:
		free(self->uniformNm_buffer);
	return 0;
}

static int
gear_createDescriptorSet(gear_t* self)
{
	assert(self);

	gears_renderer_t* renderer = self->renderer;

	self->descriptor_sets = (VkDescriptorSet*)
	                        calloc(renderer->swapchain_image_count,
	                               sizeof(VkDescriptorSet));
	if(self->descriptor_sets == NULL)
	{
		LOGE("calloc failed");
		return 0;
	}

	VkDescriptorSetAllocateInfo ds_info =
	{
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext              = NULL,
		.descriptorPool     = renderer->descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts        = &renderer->descriptor_set_layout
	};

	int i;
	for(i = 0; i < renderer->swapchain_image_count; ++i)
	{
		if(vkAllocateDescriptorSets(renderer->device, &ds_info,
		                            &self->descriptor_sets[i]) != VK_SUCCESS)
		{
			LOGE("vkAllocateDescriptorSets failed");
			goto fail_ds;
		}

		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		VkDescriptorBufferInfo db_info =
		{
			.buffer  = self->uniformMvp_buffer[i],
			.offset  = 0,
			.range   = sizeof(a3d_mat4f_t)
		};

		VkWriteDescriptorSet writes =
		{
			.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext            = NULL,
			.dstSet           = self->descriptor_sets[i],
			.dstBinding       = 0,
			.dstArrayElement  = 0,
			.descriptorCount  = 1,
			.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo       = NULL,
			.pBufferInfo      = &db_info,
			.pTexelBufferView = NULL,
		};

		vkUpdateDescriptorSets(renderer->device, 1, &writes,
		                       0, NULL);

		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		db_info.buffer    = self->uniformNm_buffer[i];
		db_info.range     = sizeof(a3d_mat4f_t);
		writes.dstBinding = 1;
		vkUpdateDescriptorSets(renderer->device, 1, &writes,
		                       0, NULL);

		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		db_info.buffer    = self->uniformColor_buffer;
		db_info.range     = sizeof(a3d_vec4f_t);
		writes.dstBinding = 2;
		vkUpdateDescriptorSets(renderer->device, 1, &writes,
		                       0, NULL);
	}

	// success
	return 1;

	// failure
	fail_ds:
		free(self->descriptor_sets);
	return 0;
}

/***********************************************************
* public                                                   *
***********************************************************/

gear_t* gear_new(struct gears_renderer_s* renderer,
                 const a3d_vec4f_t* color,
                 float inner_radius, float outer_radius, float width,
                 int teeth, float tooth_depth)
{
	gear_t* self = (gear_t*) malloc(sizeof(gear_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->renderer = renderer;

	a3d_vec4f_copy(color, &self->color);

	if(gear_createUniformMvpArray(self) == 0)
	{
		goto fail_createUniformMvp;
	}

	if(gear_createUniformNmArray(self) == 0)
	{
		goto fail_createUniformNm;
	}

	if(gear_createBuffer(self,
	                     sizeof(a3d_vec4f_t),
	                     (const void*) &self->color,
	                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                     &self->uniformColor_buffer,
	                     &self->uniformColor_memory) == 0)
	{
		goto fail_createUniformColor;
	}

	// initialize gear
	if(gear_generate(self, inner_radius, outer_radius,
	                 width, teeth, tooth_depth) == 0)
	{
		LOGE("gear_generate failed");
		goto fail_gear_generate;
	}

	if(gear_createDescriptorSet(self) == 0)
	{
		goto fail_createDescriptorSet;
	}

	// success
	return self;

	// failure
	fail_createDescriptorSet:
		vkFreeMemory(renderer->device,
		             self->cylinder_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->cylinder_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->cylinder_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->cylinder_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->outward_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->outward_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->backface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->backface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->frontface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->frontface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_vertex_buffer,
		                NULL);
	fail_gear_generate:
		vkFreeMemory(renderer->device,
		             self->uniformColor_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->uniformColor_buffer,
		                NULL);
	fail_createUniformColor:
	{
		int i;
		for(i = 0; i < renderer->swapchain_image_count; ++i)
		{
			vkFreeMemory(renderer->device,
			             self->uniformNm_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformNm_buffer[i],
			                NULL);
		}
		free(self->uniformNm_memory);
		free(self->uniformNm_buffer);
	}
	fail_createUniformNm:
	{
		int i;
		for(i = 0; i < renderer->swapchain_image_count; ++i)
		{
			vkFreeMemory(renderer->device,
			             self->uniformMvp_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformMvp_buffer[i],
			                NULL);
		}
		free(self->uniformMvp_memory);
		free(self->uniformMvp_buffer);
	}
	fail_createUniformMvp:
		free(self);
	return NULL;
}

void gear_delete(gear_t** _self)
{
	// *_self can be null
	assert(_self);

	gear_t* self = *_self;
	if(self)
	{
		gears_renderer_t* renderer = self->renderer;

		free(self->descriptor_sets);
		vkFreeMemory(renderer->device,
		             self->cylinder_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->cylinder_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->cylinder_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->cylinder_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->outward_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->outward_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->outward_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->backface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->backface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->backface_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->frontface_normal_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_normal_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->frontface_vertex_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->frontface_vertex_buffer,
		                NULL);
		vkFreeMemory(renderer->device,
		             self->uniformColor_memory,
		             NULL);
		vkDestroyBuffer(renderer->device,
		                self->uniformColor_buffer,
		                NULL);

		int i;
		for(i = 0; i < renderer->swapchain_image_count; ++i)
		{
			vkFreeMemory(renderer->device,
			             self->uniformNm_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformNm_buffer[i],
			                NULL);
		}
		free(self->uniformNm_memory);
		free(self->uniformNm_buffer);

		for(i = 0; i < renderer->swapchain_image_count; ++i)
		{
			vkFreeMemory(renderer->device,
			             self->uniformMvp_memory[i],
			             NULL);
			vkDestroyBuffer(renderer->device,
			                self->uniformMvp_buffer[i],
			                NULL);
		}
		free(self->uniformMvp_memory);
		free(self->uniformMvp_buffer);

		free(self);
		*_self = NULL;
	}
}

void gear_update(gear_t* self,
                 a3d_mat4f_t* mvp, a3d_mat4f_t* mvm)
{
	assert(self);
	assert(mvp);
	assert(mvm);

	gears_renderer_t* renderer = self->renderer;
	uint32_t          frame    = renderer->swapchain_frame;

	void* data;
	if(vkMapMemory(renderer->device,
	               self->uniformMvp_memory[frame],
	               0, sizeof(a3d_mat4f_t), 0,
	               &data) == VK_SUCCESS)
	{
		memcpy(data, (const void*) mvp,
		       sizeof(a3d_mat4f_t));
		vkUnmapMemory(renderer->device,
		              self->uniformMvp_memory[frame]);
	}
	else
	{
		LOGW("vkMapMemory failed");
	}

	a3d_mat3f_t nm;
	a3d_mat4f_normalmatrix(mvm, &nm);
	a3d_mat4f_t nm4 =
	{
		.m00 = nm.m00,
		.m10 = nm.m10,
		.m20 = nm.m20,
		.m30 = 0.0f,
		.m01 = nm.m01,
		.m11 = nm.m11,
		.m21 = nm.m21,
		.m31 = 0.0f,
		.m02 = nm.m02,
		.m12 = nm.m12,
		.m22 = nm.m22,
		.m32 = 0.0f,
		.m03 = 0.0f,
		.m13 = 0.0f,
		.m23 = 0.0f,
		.m33 = 1.0f
	};
	if(vkMapMemory(renderer->device,
	               self->uniformNm_memory[frame],
	               0, sizeof(a3d_mat4f_t), 0,
	               &data) == VK_SUCCESS)
	{
		memcpy(data, (const void*) &nm4,
		       sizeof(a3d_mat4f_t));
		vkUnmapMemory(renderer->device,
		              self->uniformNm_memory[frame]);
	}
	else
	{
		LOGW("vkMapMemory failed");
	}
}

void gear_draw(gear_t* self,
               VkCommandBuffer command_buffer)
{
	assert(self);

	gears_renderer_t* renderer;
	uint32_t          frame;
	VkDescriptorSet*  descriptor_set;
	renderer       = self->renderer;
	frame          = renderer->swapchain_frame;
	descriptor_set = &self->descriptor_sets[frame];

	vkCmdBindDescriptorSets(command_buffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        renderer->pipeline_layout,
	                        0, 1, descriptor_set,
	                        0, NULL);

	// front face
	VkDeviceSize offsets[2] = { 0, 0 };
	VkBuffer     buffers[2] =
	{
		self->frontface_vertex_buffer,
		self->frontface_normal_buffer
	};
	vkCmdBindVertexBuffers(command_buffer, 0, 2,
	                       buffers, offsets);
	vkCmdDraw(command_buffer, self->frontface_vc, 1, 0, 0);

	// back face
	buffers[0] = self->backface_vertex_buffer;
	buffers[1] = self->backface_normal_buffer;
	vkCmdBindVertexBuffers(command_buffer, 0, 2,
	                       buffers, offsets);
	vkCmdDraw(command_buffer, self->backface_vc, 1, 0, 0);

	// outward
	buffers[0] = self->outward_vertex_buffer;
	buffers[1] = self->outward_normal_buffer;
	vkCmdBindVertexBuffers(command_buffer, 0, 2,
	                       buffers, offsets);
	vkCmdDraw(command_buffer, self->outward_vc, 1, 0, 0);

	// cylinder
	buffers[0] = self->cylinder_vertex_buffer;
	buffers[1] = self->cylinder_normal_buffer;
	vkCmdBindVertexBuffers(command_buffer, 0, 2,
	                       buffers, offsets);
	vkCmdDraw(command_buffer, self->cylinder_vc, 1, 0, 0);
}
