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
	if(a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE)
	{
		goto fail_createFrontfaceV;
	}

	vkk_engine_t* engine = self->renderer->engine;
	self->frontface_vb = vkk_engine_newBuffer(engine, 0,
	                                          VKK_BUFFER_USAGE_VERTEX,
	                                          glsm->ec*sizeof(a3d_vec3f_t),
	                                          (const void*) glsm->vb);
	if(self->frontface_vb == NULL)
	{
		goto fail_createFrontfaceV;
	}
	self->frontface_vc = glsm->ec;

	self->frontface_nb = vkk_engine_newBuffer(engine, 0,
	                                          VKK_BUFFER_USAGE_VERTEX,
	                                          glsm->ec*sizeof(a3d_vec3f_t),
	                                          (const void*) glsm->nb);
	if(self->frontface_nb == NULL)
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
	if(a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE)
	{
		goto fail_createBackfaceV;
	}

	self->backface_vb = vkk_engine_newBuffer(engine, 0,
	                                         VKK_BUFFER_USAGE_VERTEX,
	                                         glsm->ec*sizeof(a3d_vec3f_t),
	                                         (const void*) glsm->vb);
	if(self->backface_vb == NULL)
	{
		goto fail_createBackfaceV;
	}
	self->backface_vc = glsm->ec;

	self->backface_nb = vkk_engine_newBuffer(engine, 0,
	                                         VKK_BUFFER_USAGE_VERTEX,
	                                         glsm->ec*sizeof(a3d_vec3f_t),
	                                         (const void*) glsm->nb);
	if(self->backface_nb == NULL)
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
	if(a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE)
	{
		goto fail_createOutwardV;
	}

	self->outward_vb = vkk_engine_newBuffer(engine, 0,
	                                        VKK_BUFFER_USAGE_VERTEX,
	                                        glsm->ec*sizeof(a3d_vec3f_t),
	                                        (const void*) glsm->vb);
	if(self->outward_vb == NULL)
	{
		goto fail_createOutwardV;
	}
	self->outward_vc = glsm->ec;

	self->outward_nb = vkk_engine_newBuffer(engine, 0,
	                                        VKK_BUFFER_USAGE_VERTEX,
	                                        glsm->ec*sizeof(a3d_vec3f_t),
	                                        (const void*) glsm->nb);
	if(self->outward_nb == NULL)
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
	if(a3d_glsm_status(glsm) != A3D_GLSM_COMPLETE)
	{
		goto fail_createCylinderV;
	}

	self->cylinder_vb = vkk_engine_newBuffer(engine, 0,
	                                         VKK_BUFFER_USAGE_VERTEX,
	                                         glsm->ec*sizeof(a3d_vec3f_t),
	                                         (const void*) glsm->vb);
	if(self->cylinder_vb == NULL)
	{
		goto fail_createCylinderV;
	}
	self->cylinder_vc = glsm->ec;

	self->cylinder_nb = vkk_engine_newBuffer(engine, 0,
	                                         VKK_BUFFER_USAGE_VERTEX,
	                                         glsm->ec*sizeof(a3d_vec3f_t),
	                                         (const void*) glsm->nb);
	if(self->cylinder_nb == NULL)
	{
		goto fail_createCylinderN;
	}

	// success
	a3d_glsm_delete(&glsm);
	return 1;

	// failure
	fail_createCylinderN:
		vkk_engine_deleteBuffer(engine, &self->cylinder_vb);
	fail_createCylinderV:
		vkk_engine_deleteBuffer(engine, &self->outward_nb);
	fail_createOutwardN:
		vkk_engine_deleteBuffer(engine, &self->outward_vb);
	fail_createOutwardV:
		vkk_engine_deleteBuffer(engine, &self->backface_nb);
	fail_createBackfaceN:
		vkk_engine_deleteBuffer(engine, &self->backface_vb);
	fail_createBackfaceV:
		vkk_engine_deleteBuffer(engine, &self->frontface_nb);
	fail_createFrontfaceN:
		vkk_engine_deleteBuffer(engine, &self->frontface_vb);
	fail_createFrontfaceV:
		a3d_glsm_delete(&glsm);
	return 0;
}

static int
gear_createDescriptorSet(gear_t* self)
{
	assert(self);

	vkk_engine_t*               engine;
	vkk_descriptorSetFactory_t* dsf;
	engine   = self->renderer->engine;
	dsf      = self->renderer->dsf;
	self->ds = vkk_engine_newDescriptorSet(engine, dsf);
	if(self->ds == NULL)
	{
		return 0;
	}

	// layout(std140, set=0, binding=0) uniform uniformMvp
	// {
	//     mat4 mvp;
	// };
	vkk_engine_writeDescriptorSetUB(engine,
	                                self->ds,
	                                self->mvp_ub,
	                                0);

	// layout(std140, set=0, binding=1) uniform uniformNm
	// {
	//     mat4 nm;
	// };
	vkk_engine_writeDescriptorSetUB(engine,
	                                self->ds,
	                                self->nm_ub,
	                                1);

	// layout(std140, set=0, binding=2) uniform uniformColor
	// {
	//     vec4 color;
	// };
	vkk_engine_writeDescriptorSetUB(engine,
	                                self->ds,
	                                self->color_ub,
	                                2);

	return 1;
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

	vkk_engine_t* engine = renderer->engine;
	self->mvp_ub = vkk_engine_newBuffer(engine, 1,
	                                    VKK_BUFFER_USAGE_UNIFORM,
	                                    sizeof(a3d_mat4f_t),
	                                    NULL);
	if(self->mvp_ub == NULL)
	{
		goto fail_createUniformMvp;
	}

	self->nm_ub = vkk_engine_newBuffer(engine, 1,
	                                   VKK_BUFFER_USAGE_UNIFORM,
	                                   sizeof(a3d_mat4f_t),
	                                   NULL);
	if(self->nm_ub == NULL)
	{
		goto fail_createUniformNm;
	}

	self->color_ub = vkk_engine_newBuffer(engine, 0,
	                                      VKK_BUFFER_USAGE_UNIFORM,
	                                      sizeof(a3d_vec4f_t),
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
		vkk_engine_deleteBuffer(engine, &self->cylinder_nb);
		vkk_engine_deleteBuffer(engine, &self->cylinder_vb);
		vkk_engine_deleteBuffer(engine, &self->outward_nb);
		vkk_engine_deleteBuffer(engine, &self->outward_vb);
		vkk_engine_deleteBuffer(engine, &self->backface_nb);
		vkk_engine_deleteBuffer(engine, &self->backface_vb);
		vkk_engine_deleteBuffer(engine, &self->frontface_nb);
		vkk_engine_deleteBuffer(engine, &self->frontface_vb);
	fail_gear_generate:
		vkk_engine_deleteBuffer(engine, &self->color_ub);
	fail_createUniformColor:
		vkk_engine_deleteBuffer(engine, &self->nm_ub);
	fail_createUniformNm:
		vkk_engine_deleteBuffer(engine, &self->mvp_ub);
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
		vkk_engine_t* engine = self->renderer->engine;
		vkk_engine_deleteDescriptorSet(engine, &self->ds);
		vkk_engine_deleteBuffer(engine, &self->cylinder_nb);
		vkk_engine_deleteBuffer(engine, &self->cylinder_vb);
		vkk_engine_deleteBuffer(engine, &self->outward_nb);
		vkk_engine_deleteBuffer(engine, &self->outward_vb);
		vkk_engine_deleteBuffer(engine, &self->backface_nb);
		vkk_engine_deleteBuffer(engine, &self->backface_vb);
		vkk_engine_deleteBuffer(engine, &self->frontface_nb);
		vkk_engine_deleteBuffer(engine, &self->frontface_vb);
		vkk_engine_deleteBuffer(engine, &self->color_ub);
		vkk_engine_deleteBuffer(engine, &self->nm_ub);
		vkk_engine_deleteBuffer(engine, &self->mvp_ub);
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

	vkk_engine_t* engine = self->renderer->engine;

	vkk_engine_updateBuffer(engine,
	                        self->mvp_ub,
	                        (const void*) mvp);

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
	vkk_engine_updateBuffer(engine,
	                        self->nm_ub,
	                        (const void*) &nm4);
}

void gear_draw(gear_t* self)
{
	assert(self);

	vkk_engine_t*         engine = self->renderer->engine;
	vkk_pipelineLayout_t* pl     = self->renderer->pl;

	vkk_engine_bindDescriptorSet(engine, pl, self->ds);

	// front face
	vkk_buffer_t* vertex_buffers[2] =
	{
		self->frontface_vb,
		self->frontface_nb,
	};
	vkk_engine_draw(engine, self->frontface_vc,
	                2, 0, NULL, vertex_buffers);

	// back face
	vertex_buffers[0] = self->backface_vb;
	vertex_buffers[1] = self->backface_nb;
	vkk_engine_draw(engine, self->backface_vc,
	                2, 0, NULL, vertex_buffers);

	// outward
	vertex_buffers[0] = self->outward_vb;
	vertex_buffers[1] = self->outward_nb;
	vkk_engine_draw(engine, self->outward_vc,
	                2, 0, NULL, vertex_buffers);

	// cylinder
	vertex_buffers[0] = self->cylinder_vb;
	vertex_buffers[1] = self->cylinder_nb;
	vkk_engine_draw(engine, self->cylinder_vc,
	                2, 0, NULL, vertex_buffers);
}
