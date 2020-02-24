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
#include "libcc/math/cc_vec3f.h"
#include "libcc/math/cc_vec4f.h"
#include "libcc/cc_log.h"
#include "libcc/cc_memory.h"
#include "gears_glsm.h"
#include "gears_renderer.h"
#include "gear.h"

/***********************************************************
* private                                                  *
***********************************************************/

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
	ASSERT(self);

	gears_glsm_t* glsm = gears_glsm_new();
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
	gears_glsm_begin(glsm);
	gears_glsm_normal3f(glsm, 0.0f, 0.0f, 1.0f);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
	}
	gears_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), dz);
	gears_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), dz);
	gears_glsm_end(glsm);

	// buffer front face
	if(gears_glsm_status(glsm) != GEARS_GLSM_COMPLETE)
	{
		goto fail_createFrontfaceV;
	}

	vkk_engine_t* engine = self->renderer->engine;
	self->frontface_vb = vkk_buffer_new(engine,
	                                    VKK_UPDATE_MODE_STATIC,
	                                    VKK_BUFFER_USAGE_VERTEX,
	                                    glsm->ec*sizeof(cc_vec3f_t),
	                                    (const void*) glsm->vb);
	if(self->frontface_vb == NULL)
	{
		goto fail_createFrontfaceV;
	}
	self->frontface_vc = glsm->ec;

	self->frontface_nb = vkk_buffer_new(engine,
	                                    VKK_UPDATE_MODE_STATIC,
	                                    VKK_BUFFER_USAGE_VERTEX,
	                                    glsm->ec*sizeof(cc_vec3f_t),
	                                    (const void*) glsm->nb);
	if(self->frontface_nb == NULL)
	{
		goto fail_createFrontfaceN;
	}

	// generate back face
	// GL_TRIANGLE_STRIP
	gears_glsm_begin(glsm);
	gears_glsm_normal3f(glsm, 0.0f, 0.0f, -1.0f);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
	}
	gears_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), -dz);
	gears_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), -dz);
	gears_glsm_end(glsm);

	// buffer back face
	if(gears_glsm_status(glsm) != GEARS_GLSM_COMPLETE)
	{
		goto fail_createBackfaceV;
	}

	self->backface_vb = vkk_buffer_new(engine,
	                                   VKK_UPDATE_MODE_STATIC,
	                                   VKK_BUFFER_USAGE_VERTEX,
	                                   glsm->ec*sizeof(cc_vec3f_t),
	                                   (const void*) glsm->vb);
	if(self->backface_vb == NULL)
	{
		goto fail_createBackfaceV;
	}
	self->backface_vc = glsm->ec;

	self->backface_nb = vkk_buffer_new(engine,
	                                   VKK_UPDATE_MODE_STATIC,
	                                   VKK_BUFFER_USAGE_VERTEX,
	                                   glsm->ec*sizeof(cc_vec3f_t),
	                                   (const void*) glsm->nb);
	if(self->backface_nb == NULL)
	{
		goto fail_createBackfaceN;
	}

	// generate outward faces of teeth
	// GL_TRIANGLE_STRIP
	// repeated vertices are necessary to achieve flat shading in ES2
	gears_glsm_begin(glsm);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		if(i > 0)
		{
			gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
			gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		}

		u = r2 * cosf(a1) - r1 * cosf(a0);
		v = r2 * sinf(a1) - r1 * sinf(a0);
		len = sqrtf(u * u + v * v);
		u /= len;
		v /= len;

		gears_glsm_normal3f(glsm, v, -u, 0.0f);
		gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a0), r1 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);

		gears_glsm_normal3f(glsm, cosf(a0), sinf(a0), 0.0f);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a1), r2 * sinf(a1), -dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);

		u = r1 * cosf(a3) - r2 * cosf(a2);
		v = r1 * sinf(a3) - r2 * sinf(a2);

		gears_glsm_normal3f(glsm, v, -u, 0.0f);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), dz);
		gears_glsm_vertex3f(glsm, r2 * cosf(a2), r2 * sinf(a2), -dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);

		gears_glsm_normal3f(glsm, cosf(a0), sinf(a0), 0.0f);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), dz);
		gears_glsm_vertex3f(glsm, r1 * cosf(a3), r1 * sinf(a3), -dz);
	}
	gears_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), dz);
	gears_glsm_vertex3f(glsm, r1 * cosf(0.0f), r1 * sinf(0.0f), -dz);
	gears_glsm_end(glsm);

	// buffer outward faces of teeth
	if(gears_glsm_status(glsm) != GEARS_GLSM_COMPLETE)
	{
		goto fail_createOutwardV;
	}

	self->outward_vb = vkk_buffer_new(engine,
	                                  VKK_UPDATE_MODE_STATIC,
	                                  VKK_BUFFER_USAGE_VERTEX,
	                                  glsm->ec*sizeof(cc_vec3f_t),
	                                  (const void*) glsm->vb);
	if(self->outward_vb == NULL)
	{
		goto fail_createOutwardV;
	}
	self->outward_vc = glsm->ec;

	self->outward_nb = vkk_buffer_new(engine,
	                                  VKK_UPDATE_MODE_STATIC,
	                                  VKK_BUFFER_USAGE_VERTEX,
	                                  glsm->ec*sizeof(cc_vec3f_t),
	                                  (const void*) glsm->nb);
	if(self->outward_nb == NULL)
	{
		goto fail_createOutwardN;
	}

	// generate inside radius cylinder
	// GL_TRIANGLE_STRIP
	gears_glsm_begin(glsm);
	for(i = 0; i < teeth; i++)
	{
		gear_angle(i, teeth, &a0, &a1, &a2, &a3);

		gears_glsm_normal3f(glsm, -cosf(a0), -sinf(a0), 0.0f);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), -dz);
		gears_glsm_vertex3f(glsm, r0 * cosf(a0), r0 * sinf(a0), dz);
	}
	gears_glsm_normal3f(glsm, -cosf(0.0f), -sinf(0.0f), 0.0f);
	gears_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), -dz);
	gears_glsm_vertex3f(glsm, r0 * cosf(0.0f), r0 * sinf(0.0f), dz);
	gears_glsm_end(glsm);

	// buffer inside radius cylinder
	if(gears_glsm_status(glsm) != GEARS_GLSM_COMPLETE)
	{
		goto fail_createCylinderV;
	}

	self->cylinder_vb = vkk_buffer_new(engine,
	                                   VKK_UPDATE_MODE_STATIC,
	                                   VKK_BUFFER_USAGE_VERTEX,
	                                   glsm->ec*sizeof(cc_vec3f_t),
	                                   (const void*) glsm->vb);
	if(self->cylinder_vb == NULL)
	{
		goto fail_createCylinderV;
	}
	self->cylinder_vc = glsm->ec;

	self->cylinder_nb = vkk_buffer_new(engine,
	                                   VKK_UPDATE_MODE_STATIC,
	                                   VKK_BUFFER_USAGE_VERTEX,
	                                   glsm->ec*sizeof(cc_vec3f_t),
	                                   (const void*) glsm->nb);
	if(self->cylinder_nb == NULL)
	{
		goto fail_createCylinderN;
	}

	// success
	gears_glsm_delete(&glsm);
	return 1;

	// failure
	fail_createCylinderN:
		vkk_buffer_delete(&self->cylinder_vb);
	fail_createCylinderV:
		vkk_buffer_delete(&self->outward_nb);
	fail_createOutwardN:
		vkk_buffer_delete(&self->outward_vb);
	fail_createOutwardV:
		vkk_buffer_delete(&self->backface_nb);
	fail_createBackfaceN:
		vkk_buffer_delete(&self->backface_vb);
	fail_createBackfaceV:
		vkk_buffer_delete(&self->frontface_nb);
	fail_createFrontfaceN:
		vkk_buffer_delete(&self->frontface_vb);
	fail_createFrontfaceV:
		gears_glsm_delete(&glsm);
	return 0;
}

static int
gear_createDescriptorSet(gear_t* self)
{
	ASSERT(self);

	gears_renderer_t* renderer = self->renderer;

	vkk_uniformAttachment_t ua_array[4] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		//     mat4 mvp;
		// };
		{
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.binding = 0,
			.buffer  = self->mvp_ub
		},
		// layout(std140, set=0, binding=1) uniform uniformNm
		// {
		//     mat4 nm;
		// };
		{
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.binding = 1,
			.buffer  = self->nm_ub
		},
		// layout(std140, set=0, binding=2) uniform uniformColor
		// {
		//     vec4 color;
		// };
		{
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.binding = 2,
			.buffer  = self->color_ub
		},
		// layout(set=0, binding=3) uniform sampler2D lava_sampler;
		{
			.type    = VKK_UNIFORM_TYPE_SAMPLER,
			.binding = 3,
			.image   = renderer->image
		},
	};

	self->us = vkk_uniformSet_new(renderer->engine,
	                              0, 4, ua_array,
	                              renderer->usf);
	if(self->us == NULL)
	{
		return 0;
	}

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gear_t* gear_new(struct gears_renderer_s* renderer,
                 const cc_vec4f_t* color,
                 float inner_radius, float outer_radius, float width,
                 int teeth, float tooth_depth)
{
	gear_t* self = (gear_t*) MALLOC(sizeof(gear_t));
	if(self == NULL)
	{
		LOGE("MALLOC failed");
		return NULL;
	}

	self->renderer = renderer;

	cc_vec4f_copy(color, &self->color);

	vkk_engine_t* engine = renderer->engine;
	self->mvp_ub = vkk_buffer_new(engine,
	                              VKK_UPDATE_MODE_DEFAULT,
	                              VKK_BUFFER_USAGE_UNIFORM,
	                              sizeof(cc_mat4f_t),
	                              NULL);
	if(self->mvp_ub == NULL)
	{
		goto fail_createUniformMvp;
	}

	self->nm_ub = vkk_buffer_new(engine,
	                             VKK_UPDATE_MODE_DEFAULT,
	                             VKK_BUFFER_USAGE_UNIFORM,
	                             sizeof(cc_mat4f_t),
	                             NULL);
	if(self->nm_ub == NULL)
	{
		goto fail_createUniformNm;
	}

	self->color_ub = vkk_buffer_new(engine,
	                                VKK_UPDATE_MODE_STATIC,
	                                VKK_BUFFER_USAGE_UNIFORM,
	                                sizeof(cc_vec4f_t),
	                                (const void*) &self->color);
	if(self->color_ub == NULL)
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
		vkk_buffer_delete(&self->cylinder_nb);
		vkk_buffer_delete(&self->cylinder_vb);
		vkk_buffer_delete(&self->outward_nb);
		vkk_buffer_delete(&self->outward_vb);
		vkk_buffer_delete(&self->backface_nb);
		vkk_buffer_delete(&self->backface_vb);
		vkk_buffer_delete(&self->frontface_nb);
		vkk_buffer_delete(&self->frontface_vb);
	fail_gear_generate:
		vkk_buffer_delete(&self->color_ub);
	fail_createUniformColor:
		vkk_buffer_delete(&self->nm_ub);
	fail_createUniformNm:
		vkk_buffer_delete(&self->mvp_ub);
	fail_createUniformMvp:
		FREE(self);
	return NULL;
}

void gear_delete(gear_t** _self)
{
	// *_self can be null
	ASSERT(_self);

	gear_t* self = *_self;
	if(self)
	{
		vkk_uniformSet_delete(&self->us);
		vkk_buffer_delete(&self->cylinder_nb);
		vkk_buffer_delete(&self->cylinder_vb);
		vkk_buffer_delete(&self->outward_nb);
		vkk_buffer_delete(&self->outward_vb);
		vkk_buffer_delete(&self->backface_nb);
		vkk_buffer_delete(&self->backface_vb);
		vkk_buffer_delete(&self->frontface_nb);
		vkk_buffer_delete(&self->frontface_vb);
		vkk_buffer_delete(&self->color_ub);
		vkk_buffer_delete(&self->nm_ub);
		vkk_buffer_delete(&self->mvp_ub);
		FREE(self);
		*_self = NULL;
	}
}

void gear_update(gear_t* self,
                 vkk_renderer_t* renderer,
                 cc_mat4f_t* mvp, cc_mat4f_t* mvm)
{
	ASSERT(self);
	ASSERT(renderer);
	ASSERT(mvp);
	ASSERT(mvm);

	vkk_renderer_updateBuffer(renderer,
	                          self->mvp_ub,
	                          sizeof(cc_mat4f_t),
	                          (const void*) mvp);

	cc_mat3f_t nm;
	cc_mat4f_normalmatrix(mvm, &nm);
	cc_mat4f_t nm4 =
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
	vkk_renderer_updateBuffer(renderer,
	                          self->nm_ub,
	                          sizeof(cc_mat4f_t),
	                          (const void*) &nm4);
}

void gear_draw(gear_t* self, vkk_renderer_t* renderer)
{
	ASSERT(self);
	ASSERT(renderer);

	vkk_pipelineLayout_t* pl = self->renderer->pl;

	vkk_renderer_bindUniformSets(renderer, pl, 1, &self->us);

	// front face
	vkk_buffer_t* vertex_buffers[2] =
	{
		self->frontface_vb,
		self->frontface_nb,
	};
	vkk_renderer_draw(renderer, self->frontface_vc,
	                  2, vertex_buffers);

	// back face
	vertex_buffers[0] = self->backface_vb;
	vertex_buffers[1] = self->backface_nb;
	vkk_renderer_draw(renderer, self->backface_vc,
	                  2, vertex_buffers);

	// outward
	vertex_buffers[0] = self->outward_vb;
	vertex_buffers[1] = self->outward_nb;
	vkk_renderer_draw(renderer, self->outward_vc,
	                  2, vertex_buffers);

	// cylinder
	vertex_buffers[0] = self->cylinder_vb;
	vertex_buffers[1] = self->cylinder_nb;
	vkk_renderer_draw(renderer, self->cylinder_vc,
	                  2, vertex_buffers);
}
