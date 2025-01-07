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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/math/cc_mat4f.h"
#include "libcc/math/cc_vec4f.h"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "libbfs/bfs_file.h"
#include "texgz/texgz_png.h"
#include "texgz/texgz_tex.h"
#include "gears_renderer.h"

/***********************************************************
* private                                                  *
***********************************************************/

// gear colors
static const cc_vec4f_t RED   = { .r=0.8f, .g=0.1f, .b=0.0f, .a=1.0f };
static const cc_vec4f_t GREEN = { .r=0.0f, .g=0.8f, .b=0.2f, .a=1.0f };
static const cc_vec4f_t BLUE  = { .r=0.2f, .g=0.2f, .b=1.0f, .a=1.0f };

static void gears_renderer_step(gears_renderer_t* self)
{
	ASSERT(self);

	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
	// Vulkan uses a top-left left origin while OpenGL
	// uses a bottom-left origin so the frustum top and
	// bottom should be swapped to compensate
	if(self->draw_h > self->draw_w)
	{
		float a = self->draw_h/self->draw_w;
		cc_mat4f_frustum(&self->pm, 1, -1.0f, 1.0f, a, -a, 5.0f, 1000.0f);
	}
	else
	{
		float a = self->draw_w/self->draw_h;
		cc_mat4f_frustum(&self->pm, 1, -a, a, 1.0f, -1.0f, 5.0f, 1000.0f);
	}
	cc_mat4f_translate(&self->mvm, 1, 0.0f, 0.0f, -40.0f);

	// glxgears: event_loop
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_scale(&self->mvm, 0, self->view_scale, self->view_scale, self->view_scale);
	cc_mat4f_rotateq(&self->mvm, 0, &self->view_q);

	// Gear1
	cc_mat4f_t mvp;
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_translate(&self->mvm, 0, -3.0f, -2.0f, 0.0f);
	cc_mat4f_rotate(&self->mvm, 0, self->angle, 0.0f, 0.0f, 1.0f);
	cc_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear1, self->rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear2
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_translate(&self->mvm, 0, 3.1f, -2.0f, 0.0f);
	cc_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 9.0f, 0.0f, 0.0f, 1.0f);
	cc_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear2, self->rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear3
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_translate(&self->mvm, 0, -3.1f, 4.2f, 0.0f);
	cc_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 25.0f, 0.0f, 0.0f, 1.0f);
	cc_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear3, self->rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	// query fps
	int fps = vkk_renderer_fps(self->rend);
	if(fps == 0)
	{
		fps = 60;
	}

	// make the gears rotate at a constant rate
	// red gear rotates at 1 revolution per 7 seconds
	self->angle += 360.0f/(7.0f*fps);
	if(self->angle >= 360.0f)
	{
		self->angle -= 360.0f;
	}
}

static int
gears_renderer_newUniformSetFactory(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_uniformBinding_t ub_array[4] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		{
			.binding = 0,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_VS,
		},
		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		{
			.binding = 1,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_VS,
		},
		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		{
			.binding = 2,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_FS,
		},
		// layout(set=0, binding=3) uniform sampler2D lava_sampler;
		{
			.binding = 3,
			.type    = VKK_UNIFORM_TYPE_IMAGE,
			.stage   = VKK_STAGE_FS,
			.si      =
			{
				VKK_SAMPLER_FILTER_LINEAR,
				VKK_SAMPLER_FILTER_LINEAR,
				VKK_SAMPLER_MIPMAP_MODE_NEAREST,
			}
		}
	};

	self->usf = vkk_uniformSetFactory_new(self->engine,
	                                      VKK_UPDATE_MODE_ASYNCHRONOUS,
	                                      4, ub_array);
	if(self->usf == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newPipelineLayout(gears_renderer_t* self)
{
	ASSERT(self);

	self->pl = vkk_pipelineLayout_new(self->engine,
	                                  1, &self->usf);
	if(self->pl == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newGraphicsPipeline(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_vertexBufferInfo_t vbi[2] =
	{
		// layout(location=0) in vec3 vertex;
		{
			.location   = 0,
			.components = 3,
			.format     = VKK_VERTEX_FORMAT_FLOAT
		},
		// layout(location=1) in vec3 normal;
		{
			.location   = 1,
			.components = 3,
			.format     = VKK_VERTEX_FORMAT_FLOAT
		}
	};

	vkk_graphicsPipelineInfo_t gpi =
	{
		.renderer          = self->rend,
		.pl                = self->pl,
		.vs                = "shaders/vert.spv",
		.fs                = "shaders/frag.spv",
		.vb_count          = 2,
		.vbi               = vbi,
		.primitive         = VKK_PRIMITIVE_TRIANGLE_STRIP,
		.primitive_restart = 0,
		.cull_back         = 0,
		.depth_test        = 1,
		.depth_write       = 1,
		.blend_mode        = 0
	};

	self->gp = vkk_graphicsPipeline_new(self->engine,
	                                    &gpi);
	if(self->gp == NULL)
	{
		return 0;
	}

	return 1;
}

static int
gears_renderer_newImage(gears_renderer_t* self)
{
	ASSERT(self);

	// check for the required image caps
	vkk_imageCaps_t caps;
	vkk_engine_imageCaps(self->engine,
	                     VKK_IMAGE_FORMAT_RGBA8888, &caps);
	if((caps.texture == 0) || (caps.mipmap == 0) ||
	   (caps.filter_linear == 0))
	{
		LOGE("invalid texture=%i, mipmap=%i, filter_linear=%i",
		     (int) caps.texture, (int) caps.mipmap,
		     (int) caps.filter_linear);
		return 0;
	}

	const char* resource_path;
	resource_path = vkk_engine_internalPath(self->engine);

	char resource[256];
	snprintf(resource, 256, "%s/resource.bfs", resource_path);

	bfs_file_t* bfs;
	bfs = bfs_file_open(resource, 1, BFS_MODE_RDONLY);
	if(bfs == NULL)
	{
		return 0;
	}

	size_t size = 0;
	void*  data = NULL;
	if(bfs_file_blobGet(bfs, 0, "textures/lava.png",
	                    &size, &data) == 0)
	{
		goto fail_get;
	}

	// check for empty data
	if(size == 0)
	{
		goto fail_empty;
	}

	texgz_tex_t* tex;
	tex = texgz_png_importd(size, data);
	if(tex == NULL)
	{
		goto fail_import;
	}

	if(texgz_tex_convert(tex, TEXGZ_UNSIGNED_BYTE,
	                     TEXGZ_RGBA) == 0)
	{
		goto fail_convert;
	}

	self->image = vkk_image_new(self->engine,
	                            (uint32_t) tex->width,
	                            (uint32_t) tex->height,
	                            1, VKK_IMAGE_FORMAT_RGBA8888,
	                            1, VKK_STAGE_FS,
	                            tex->pixels);
	if(self->image == NULL)
	{
		goto fail_image;
	}

	texgz_tex_delete(&tex);
	FREE(data);
	bfs_file_close(&bfs);

	// success
	return 1;

	// failure
	fail_image:
	fail_convert:
		texgz_tex_delete(&tex);
	fail_import:
	fail_empty:
		FREE(data);
	fail_get:
		bfs_file_close(&bfs);
	return 0;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_renderer_t*
gears_renderer_new(vkk_engine_t* engine,
                   vkk_renderer_t* rend)
{
	ASSERT(engine);
	ASSERT(rend);

	gears_renderer_t* self;
	self = (gears_renderer_t*)
	       CALLOC(1, sizeof(gears_renderer_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->view_scale = 1.0f;
	self->engine     = engine;
	self->rend       = rend;

	if(gears_renderer_newUniformSetFactory(self) == 0)
	{
		goto fail_usf;
	}

	if(gears_renderer_newPipelineLayout(self) == 0)
	{
		goto fail_pl;
	}

	if(gears_renderer_newGraphicsPipeline(self) == 0)
	{
		goto fail_gp;
	}

	if(gears_renderer_newImage(self) == 0)
	{
		goto fail_image;
	}

	cc_quaternion_t qx;
	cc_quaternion_t qy;
	cc_quaternion_loadeuler(&qx, 20.0f,  0.0f, 0.0f);
	cc_quaternion_loadeuler(&qy,  0.0f, 30.0f, 0.0f);
	cc_quaternion_rotateq_copy(&qy, &qx, &self->view_q);

	cc_mat4f_identity(&self->pm);
	cc_mat4f_identity(&self->mvm);

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

	self->mvm_stack = cc_stack4f_new();
	if(self->mvm_stack == NULL)
	{
		goto fail_stack;
	}

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
		vkk_image_delete(&self->image);
	fail_image:
		vkk_graphicsPipeline_delete(&self->gp);
	fail_gp:
		vkk_pipelineLayout_delete(&self->pl);
	fail_pl:
		vkk_uniformSetFactory_delete(&self->usf);
	fail_usf:
		FREE(self);
	return NULL;
}

void gears_renderer_delete(gears_renderer_t** _self)
{
	// *_self can be null
	ASSERT(_self);

	gears_renderer_t* self = *_self;
	if(self)
	{
		cc_stack4f_delete(&self->mvm_stack);
		gear_delete(&self->gear3);
		gear_delete(&self->gear2);
		gear_delete(&self->gear1);

		vkk_image_delete(&self->image);
		vkk_graphicsPipeline_delete(&self->gp);
		vkk_pipelineLayout_delete(&self->pl);
		vkk_uniformSetFactory_delete(&self->usf);
		FREE(self);
		*_self = NULL;
	}
}

void gears_renderer_draw(gears_renderer_t* self,
                         float draw_w, float draw_h)
{
	ASSERT(self);

	self->draw_w = draw_w;
	self->draw_h = draw_h;

	gears_renderer_step(self);

	vkk_renderer_bindGraphicsPipeline(self->rend, self->gp);

	gear_draw(self->gear1, self->rend);
	gear_draw(self->gear2, self->rend);
	gear_draw(self->gear3, self->rend);
}

void gears_renderer_rotate(gears_renderer_t* self,
                           float dx, float dy)
{
	ASSERT(self);

	// rotating around x-axis is equivalent to moving up-and-down on touchscreen
	// rotating around y-axis is equivalent to moving left-and-right on touchscreen
	// 360 degrees is equivalent to moving completly across the touchscreen
	if((self->draw_w > 0.0f) && (self->draw_h > 0.0f))
	{
		float w  = self->draw_w;
		float h  = self->draw_h;
		float rx = 360.0f * dy / h;
		float ry = 360.0f * dx / w;
		cc_quaternion_t q;
		cc_quaternion_loadeuler(&q, rx, ry, 0.0f);
		cc_quaternion_rotateq(&q, &self->view_q);
		cc_quaternion_copy(&q, &self->view_q);
	}
}

void gears_renderer_scale(gears_renderer_t* self,
                          float scale)
{
	ASSERT(self);

	self->view_scale *= scale;

	// clamp view_scale
	float min = 0.25f;
	float max = 2.0f;
	if(self->view_scale < min)
	{
		self->view_scale = min;
	}
	else if(self->view_scale >= max)
	{
		self->view_scale = max;
	}
}
