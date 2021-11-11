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
#include "libcc/cc_timestamp.h"
#include "libpak/pak_file.h"
#include "libvkk/vkk_platform.h"
#include "texgz/texgz_png.h"
#include "texgz/texgz_tex.h"
#include "gears_renderer.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const float XYUV[] =
{
	-1.0f,  1.0f, 0.0f, 0.0f,   // top-left
	-1.0f, -1.0f, 0.0f, 1.0f,   // bottom-left
	 1.0f,  1.0f, 1.0f, 0.0f,   // top-right
	 1.0f, -1.0f, 1.0f, 1.0f,   // bottom-right
};


// gear colors
static const cc_vec4f_t RED   = { .r=0.8f, .g=0.1f, .b=0.0f, .a=1.0f };
static const cc_vec4f_t GREEN = { .r=0.0f, .g=0.8f, .b=0.2f, .a=1.0f };
static const cc_vec4f_t BLUE  = { .r=0.2f, .g=0.2f, .b=1.0f, .a=1.0f };

static void gears_renderer_step(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_renderer_surfaceSize(self->draw_rend,
	                         &self->screen_w,
	                         &self->screen_h);

	// update screen, viewport and scissor
	float screen_x = 0.0f;
	float screen_y = 0.0f;
	float screen_w = (float) self->screen_w;
	float screen_h = (float) self->screen_h;
	if((self->content_rect_width  > 0) &&
	   (self->content_rect_height > 0) &&
	   (self->content_rect_width  <= self->screen_w) &&
	   (self->content_rect_height <= self->screen_h))
	{
		screen_x = (float) self->content_rect_left;
		screen_y = (float) self->content_rect_top;
		screen_w = (float) self->content_rect_width;
		screen_h = (float) self->content_rect_height;

		vkk_renderer_viewport(self->draw_rend,
		                      screen_x, screen_y,
		                      screen_w, screen_h);
		vkk_renderer_scissor(self->draw_rend,
		                     self->content_rect_left,
		                     self->content_rect_top,
		                     self->content_rect_width,
		                     self->content_rect_height);
	}

	// https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
	// Vulkan uses a top-left left origin while OpenGL
	// uses a bottom-left origin so the frustum top and
	// bottom should be swapped to compensate
	if(screen_h > screen_w)
	{
		float a = screen_h/screen_w;
		cc_mat4f_frustum(&self->pm, 1, -1.0f, 1.0f, a, -a, 5.0f, 60.0f);
	}
	else
	{
		float a = screen_w/screen_h;
		cc_mat4f_frustum(&self->pm, 1, -a, a, 1.0f, -1.0f, 5.0f, 60.0f);
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
	gear_update(self->gear1, self->draw_rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear2
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_translate(&self->mvm, 0, 3.1f, -2.0f, 0.0f);
	cc_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 9.0f, 0.0f, 0.0f, 1.0f);
	cc_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear2, self->draw_rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	// Gear3
	cc_stack4f_push(self->mvm_stack, &self->mvm);
	cc_mat4f_translate(&self->mvm, 0, -3.1f, 4.2f, 0.0f);
	cc_mat4f_rotate(&self->mvm, 0, -2.0f * self->angle - 25.0f, 0.0f, 0.0f, 1.0f);
	cc_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
	gear_update(self->gear3, self->draw_rend, &mvp, &self->mvm);
	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	cc_stack4f_pop(self->mvm_stack, &self->mvm);

	double t     = cc_timestamp();
	double dt0   = t - self->t0;
	++self->frames;

	// don't update fps every frame
	if(dt0 >= 1.0)
	{
		float seconds  = (float) dt0;
		self->last_fps = ((float) self->frames)/seconds;

		//LOGI("%i frames in %.2lf seconds = %.2lf FPS", self->frames, seconds, self->last_fps);
		gears_overlay_updateFps(self->overlay, (int) (self->last_fps + 0.5f));

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
	ASSERT(self);

	// rotating around x-axis is equivalent to moving up-and-down on touchscreen
	// rotating around y-axis is equivalent to moving left-and-right on touchscreen
	// 360 degrees is equivalent to moving completly across the touchscreen
	if(self->screen_w && self->screen_h)
	{
		float w  = (float) self->screen_w;
		float h  = (float) self->screen_h;
		float rx = 360.0f * dy / h;
		float ry = 360.0f * dx / w;
		cc_quaternion_t q;
		cc_quaternion_loadeuler(&q, rx, ry, 0.0f);
		cc_quaternion_rotateq(&self->view_q, &q);
	}
}

static void gears_renderer_scale(gears_renderer_t* self,
                                 float scale)
{
	ASSERT(self);

	// scale range
	float min = 0.25f;
	float max = 2.0f;

	self->view_scale *= scale;
	if(self->view_scale < min)  self->view_scale = min;
	if(self->view_scale >= max) self->view_scale = max;
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
	snprintf(resource, 256, "%s/resource.pak", resource_path);

	pak_file_t* pak;
	pak = pak_file_open(resource, PAK_FLAG_READ);
	if(pak == NULL)
	{
		return 0;
	}

	int size = pak_file_seek(pak, "textures/lava.png");
	if(size == 0)
	{
		LOGE("invalid lava.png");
		pak_file_close(&pak);
		return 0;
	}

	texgz_tex_t* tex;
	tex = texgz_png_importf(pak->f, (size_t) size);
	if(tex == NULL)
	{
		pak_file_close(&pak);
		return 0;
	}
	pak_file_close(&pak);

	if(texgz_tex_convert(tex, TEXGZ_UNSIGNED_BYTE,
	                     TEXGZ_RGBA) == 0)
	{
		goto fail_convert;
	}

	self->draw_lava = vkk_image_new(self->engine,
	                                (uint32_t) tex->width,
	                                (uint32_t) tex->height,
	                                1, VKK_IMAGE_FORMAT_RGBA8888,
	                                1, VKK_STAGE_FS,
	                                tex->pixels);
	if(self->draw_lava == NULL)
	{
		goto fail_draw_lava;
	}

	texgz_tex_delete(&tex);

	// success
	return 1;

	// failure
	fail_draw_lava:
	fail_convert:
		texgz_tex_delete(&tex);
	return 0;
}

int gears_renderer_begin(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_renderer_t* default_rend;
	default_rend = vkk_engine_defaultRenderer(self->engine);

	// get the default renderer size
	uint32_t w1;
	uint32_t h1;
	vkk_renderer_surfaceSize(default_rend, &w1, &h1);

	// compute the scalar size if needed
	// ensuring it is an even size
	uint32_t w2 = w1;
	uint32_t h2 = h1;
	if(self->scaler_mode == GEARS_SCALER_UPSCALE)
	{
		w2 = (w1/2) + (w1%2);
		h2 = (h1/2) + (h1%2);
	}
	else if(self->scaler_mode == GEARS_SCALER_SUPERSCALE)
	{
		w2 = w1*2;
		h2 = h1*2;
	}

	// validate the current draw renderer
	int reset = 0;
	if(self->scaler_mode)
	{
		if((self->draw_rend == NULL) ||
		   (self->draw_rend == default_rend))
		{
			reset = 1;
		}
		else
		{
			// get the draw renderer size
			uint32_t w3;
			uint32_t h3;
			vkk_renderer_surfaceSize(self->draw_rend, &w3, &h3);

			// validate the size
			if((w2 != w3) || (h2 != h3))
			{
				reset = 1;
			}
		}
	}
	else if(self->draw_rend != default_rend)
	{
		reset = 1;
	}

	// reset the graphics state
	if(reset)
	{
		vkk_renderer_delete(&self->draw_rend);
		vkk_graphicsPipeline_delete(&self->draw_gp);

		// note that vkk_renderer_delete will not set to NULL
		// for the default renderer
		self->draw_rend = NULL;
	}

	// setup renderer
	if(self->scaler_mode == 0)
	{
		self->draw_rend = default_rend;
	}
	else if(self->scaler_mode && (self->draw_rend == NULL))
	{
		self->draw_rend =
			vkk_renderer_newImageStream(default_rend,
			                            w2, h2,
			                            VKK_IMAGE_FORMAT_RGBA8888,
			                            0, VKK_STAGE_FS);
	}

	// check renderer
	if(self->draw_rend == NULL)
	{
		return 0;
	}

	// setup graphics pipeline
	if(self->draw_gp == NULL)
	{
		// recreate the graphics pipeline
		vkk_vertexBufferInfo_t draw_vbi[2] =
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

		vkk_graphicsPipelineInfo_t draw_gpi =
		{
			.renderer          = self->draw_rend,
			.pl                = self->draw_pl,
			.vs                = "shaders/draw_vert.spv",
			.fs                = "shaders/draw_frag.spv",
			.vb_count          = 2,
			.vbi               = draw_vbi,
			.primitive         = VKK_PRIMITIVE_TRIANGLE_STRIP,
			.primitive_restart = 0,
			.cull_back         = 0,
			.depth_test        = 1,
			.depth_write       = 1,
			.blend_mode        = 0
		};

		self->draw_gp = vkk_graphicsPipeline_new(self->engine,
		                                         &draw_gpi);
		if(self->draw_gp == NULL)
		{
			return 0;
		}
	}

	// optionally begin rendering to image stream
	if(self->scaler_mode)
	{
		float red[4] =
		{
			1.0f, 0.0f, 0.0f, 1.0f
		};

		float green[4] =
		{
			0.0f, 1.0f, 0.0f, 1.0f
		};

		float* clear_color = red;
		if(self->scaler_mode == GEARS_SCALER_SUPERSCALE)
		{
			clear_color = green;
		}

		self->draw_image =
			vkk_renderer_beginImageStream(self->draw_rend,
			                              VKK_RENDERER_MODE_DRAW,
			                              clear_color);
		if(self->draw_image == NULL)
		{
			return 0;
		}
	}

	return 1;
}

void gears_renderer_end(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_renderer_t* default_rend;
	default_rend = vkk_engine_defaultRenderer(self->engine);

	// check if upscaling was used
	if(self->draw_image == NULL)
	{
		return;
	}

	// end upscaling
	vkk_renderer_end(self->draw_rend);

	vkk_renderer_bindGraphicsPipeline(default_rend,
	                                  self->scaler_gp);

	uint32_t w;
	uint32_t h;
	vkk_renderer_surfaceSize(default_rend, &w, &h);

	cc_mat4f_t mvp;
	cc_mat4f_orthoVK(&mvp, 1, -1.0f, 1.0f,
	                 -1.0f, 1.0f, 0.0f, 2.0f);
	vkk_renderer_updateBuffer(default_rend,
	                          self->scaler_ub00_mvp,
	                          sizeof(cc_mat4f_t),
	                          (const void*) &mvp);

	vkk_uniformAttachment_t ua01 =
	{
		.binding = 1,
		.type    = VKK_UNIFORM_TYPE_IMAGE_REF,
		.image   = self->draw_image
	};
	vkk_renderer_updateUniformSetRefs(default_rend,
	                                  self->scaler_us0,
	                                  1, &ua01);

	vkk_uniformSet_t* us_array0[] =
	{
		self->scaler_us0,
	};
	vkk_renderer_bindUniformSets(default_rend, 1, us_array0);

	vkk_renderer_draw(default_rend, 4, 1,
	                  &self->scaler_vb_xyuv);

	// reset upscaling
	self->draw_image = NULL;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_renderer_t*
gears_renderer_new(vkk_engine_t* engine)
{
	ASSERT(engine);

	vkk_renderer_t* default_rend;
	default_rend = vkk_engine_defaultRenderer(engine);

	gears_renderer_t* self;
	self = (gears_renderer_t*)
	       CALLOC(1, sizeof(gears_renderer_t));
	if(self == NULL)
	{
		LOGE("CALLOC failed");
		return NULL;
	}

	self->view_scale  = 1.0f;
	self->t0          = cc_timestamp();
	self->density     = 1.0f;
	self->touch_state = GEARS_TOUCH_STATE_INIT;
	self->touch_ds    = 1.0f;
	self->escape_t0   = cc_timestamp();
	self->engine      = engine;

	vkk_uniformBinding_t draw_ub_array[] =
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

	self->draw_usf = vkk_uniformSetFactory_new(self->engine,
	                                           VKK_UPDATE_MODE_ASYNCHRONOUS,
	                                           4, draw_ub_array);
	if(self->draw_usf == NULL)
	{
		goto fail_draw_usf;
	}

	self->draw_pl = vkk_pipelineLayout_new(self->engine,
	                                       1, &self->draw_usf);
	if(self->draw_pl == NULL)
	{
		goto fail_draw_pl;
	}

	if(gears_renderer_newImage(self) == 0)
	{
		goto fail_draw_lava;
	}

	vkk_uniformBinding_t scaler_ub_array0[] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		// {
		// 	mat4 mvp;
		// };
		{
			.binding = 0,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.stage   = VKK_STAGE_VS,
		},
		// layout(set=0, binding=1) uniform sampler2D image;
		{
			.binding = 1,
			.type    = VKK_UNIFORM_TYPE_IMAGE_REF,
			.stage   = VKK_STAGE_FS,
			.si      =
			{
				VKK_SAMPLER_FILTER_LINEAR,
				VKK_SAMPLER_FILTER_LINEAR,
				VKK_SAMPLER_MIPMAP_MODE_NEAREST,
			}
		}
	};

	self->scaler_usf0 = vkk_uniformSetFactory_new(self->engine,
	                                                VKK_UPDATE_MODE_ASYNCHRONOUS,
	                                                2, scaler_ub_array0);
	if(self->scaler_usf0 == NULL)
	{
		goto fail_scaler_usf0;
	}

	self->scaler_pl = vkk_pipelineLayout_new(self->engine,
	                                           1, &self->scaler_usf0);
	if(self->scaler_pl == NULL)
	{
		goto fail_scaler_pl;
	}

	vkk_vertexBufferInfo_t scaler_vbi[1] =
	{
		// layout(location=0) in vec4 xyuv;
		{
			.location   = 0,
			.components = 4,
			.format     = VKK_VERTEX_FORMAT_FLOAT
		},
	};

	vkk_graphicsPipelineInfo_t scaler_gpi =
	{
		.renderer          = default_rend,
		.pl                = self->scaler_pl,
		.vs                = "shaders/scaler_vert.spv",
		.fs                = "shaders/scaler_frag.spv",
		.vb_count          = 1,
		.vbi               = scaler_vbi,
		.primitive         = VKK_PRIMITIVE_TRIANGLE_STRIP,
		.primitive_restart = 0,
		.cull_back         = 0,
		.depth_test        = 0,
		.depth_write       = 0,
		.blend_mode        = 0
	};

	self->scaler_gp = vkk_graphicsPipeline_new(self->engine,
	                                             &scaler_gpi);
	if(self->scaler_gp == NULL)
	{
		goto fail_scaler_gp;
	}

	self->scaler_vb_xyuv = vkk_buffer_new(self->engine,
	                                        VKK_UPDATE_MODE_STATIC,
	                                        VKK_BUFFER_USAGE_VERTEX,
	                                        16*sizeof(float),
	                                        (const void*) XYUV);
	if(self->scaler_vb_xyuv == NULL)
	{
		goto fail_scaler_vb_xyuv;
	}

	self->scaler_ub00_mvp = vkk_buffer_new(self->engine,
	                                         VKK_UPDATE_MODE_ASYNCHRONOUS,
	                                         VKK_BUFFER_USAGE_UNIFORM,
	                                         sizeof(cc_mat4f_t), NULL);
	if(self->scaler_ub00_mvp == NULL)
	{
		goto fail_scaler_ub00_mvp;
	}

	vkk_uniformAttachment_t ua_array0[] =
	{
		// layout(std140, set=0, binding=0) uniform uniformMvp
		{
			.binding = 0,
			.type    = VKK_UNIFORM_TYPE_BUFFER,
			.buffer  = self->scaler_ub00_mvp
		},
	};

	self->scaler_us0 = vkk_uniformSet_new(self->engine, 0, 1,
	                                        ua_array0,
	                                        self->scaler_usf0);
	if(self->scaler_us0 == NULL)
	{
		goto fail_scaler_us0;
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

	self->overlay = gears_overlay_new(self);
	if(self->overlay == NULL)
	{
		goto fail_overlay;
	}

	// success
	return self;

	// failure
	fail_overlay:
		cc_stack4f_delete(&self->mvm_stack);
	fail_stack:
		gear_delete(&self->gear3);
	fail_gear3:
		gear_delete(&self->gear2);
	fail_gear2:
		gear_delete(&self->gear1);
	fail_gear1:
		vkk_uniformSet_delete(&self->scaler_us0);
	fail_scaler_us0:
		vkk_buffer_delete(&self->scaler_ub00_mvp);
	fail_scaler_ub00_mvp:
		vkk_buffer_delete(&self->scaler_vb_xyuv);
	fail_scaler_vb_xyuv:
		vkk_graphicsPipeline_delete(&self->scaler_gp);
	fail_scaler_gp:
		vkk_pipelineLayout_delete(&self->scaler_pl);
	fail_scaler_pl:
		vkk_uniformSetFactory_delete(&self->scaler_usf0);
	fail_scaler_usf0:
		vkk_image_delete(&self->draw_lava);
	fail_draw_lava:
		vkk_pipelineLayout_delete(&self->draw_pl);
	fail_draw_pl:
		vkk_uniformSetFactory_delete(&self->draw_usf);
	fail_draw_usf:
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
		gears_overlay_delete(&self->overlay);
		cc_stack4f_delete(&self->mvm_stack);
		gear_delete(&self->gear3);
		gear_delete(&self->gear2);
		gear_delete(&self->gear1);

		vkk_uniformSet_delete(&self->scaler_us0);
		vkk_buffer_delete(&self->scaler_ub00_mvp);
		vkk_buffer_delete(&self->scaler_vb_xyuv);
		vkk_graphicsPipeline_delete(&self->scaler_gp);
		vkk_pipelineLayout_delete(&self->scaler_pl);
		vkk_uniformSetFactory_delete(&self->scaler_usf0);
		vkk_image_delete(&self->draw_lava);
		vkk_graphicsPipeline_delete(&self->draw_gp);
		vkk_pipelineLayout_delete(&self->draw_pl);
		vkk_uniformSetFactory_delete(&self->draw_usf);
		vkk_renderer_delete(&self->draw_rend);
		FREE(self);
		*_self = NULL;
	}
}

void gears_renderer_exit(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_engine_platformCmd(self->engine,
	                       VKK_PLATFORM_CMD_EXIT, NULL);
}

void gears_renderer_loadURL(gears_renderer_t* self,
                            const char* url)
{
	ASSERT(self);
	ASSERT(url);

	vkk_engine_platformCmd(self->engine,
	                       VKK_PLATFORM_CMD_LOADURL, url);
}

void gears_renderer_playClick(void* ptr)
{
	ASSERT(ptr);

	gears_renderer_t* self = (gears_renderer_t*) ptr;

	vkk_engine_platformCmd(self->engine,
	                       VKK_PLATFORM_CMD_PLAY_CLICK, NULL);
}

void gears_renderer_density(gears_renderer_t* self,
                            float density)
{
	ASSERT(self);

	self->density = density;
}

void gears_renderer_draw(gears_renderer_t* self)
{
	ASSERT(self);

	vkk_renderer_t* default_rend;
	default_rend = vkk_engine_defaultRenderer(self->engine);

	float clear_color[4] =
	{
		0.0f, 1.0f, 1.0f, 1.0f
	};
	if(vkk_renderer_beginDefault(default_rend,
	                             VKK_RENDERER_MODE_DRAW,
	                             clear_color) == 0)
	{
		return;
	}

	// draw scene using draw renderer
	if(gears_renderer_begin(self))
	{
		gears_renderer_step(self);

		vkk_renderer_bindGraphicsPipeline(self->draw_rend,
		                                  self->draw_gp);

		gear_draw(self->gear1, self->draw_rend);
		gear_draw(self->gear2, self->draw_rend);
		gear_draw(self->gear3, self->draw_rend);
		gears_renderer_end(self);
	}

	// draw overlay using default renderer
	gears_overlay_draw(self->overlay, self->density);

	vkk_renderer_end(default_rend);
}

void gears_renderer_touch(gears_renderer_t* self,
                          int action, int count, double ts,
                          float x0, float y0,
                          float x1, float y1,
                          float x2, float y2,
                          float x3, float y3)
{
	ASSERT(self);

	if(action == GEARS_TOUCH_ACTION_UP)
	{
		if(self->touch_state == GEARS_TOUCH_STATE_OVERLAY)
		{
			gears_overlay_pointerUp(self->overlay, x0, y0, ts);
		}

		// Do nothing
		self->touch_state = GEARS_TOUCH_STATE_INIT;
	}
	else if(count == 1)
	{
		if((self->touch_state == GEARS_TOUCH_STATE_INIT) &&
		   (action == GEARS_TOUCH_ACTION_DOWN) &&
		   gears_overlay_pointerDown(self->overlay, x0, y0, ts))
		{
			self->touch_state = GEARS_TOUCH_STATE_OVERLAY;
		}
		else if(self->touch_state == GEARS_TOUCH_STATE_OVERLAY)
		{
			gears_overlay_pointerMove(self->overlay, x0, y0, ts);
		}
		else if(self->touch_state == GEARS_TOUCH_STATE_ROTATE)
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
		if(self->touch_state == GEARS_TOUCH_STATE_OVERLAY)
		{
			// ignore
		}
		else if(self->touch_state == GEARS_TOUCH_STATE_ZOOM)
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
	ASSERT(self);

	if(keycode == VKK_KEYCODE_ESCAPE)
	{
		if(gears_overlay_escape(self->overlay))
		{
			// ignore
		}
		else
		{
			// double tap back to exit
			double t1 = cc_timestamp();
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
	else if(keycode == '1')
	{
		self->scaler_mode = 0;
	}
	else if(keycode == '2')
	{
		self->scaler_mode = 1;
	}
	else if(keycode == '3')
	{
		self->scaler_mode = 2;
	}
}

void gears_renderer_contentRect(gears_renderer_t* self,
                                uint32_t t, uint32_t l,
                                uint32_t b, uint32_t r)
{
	ASSERT(self);

	self->content_rect_top    = t;
	self->content_rect_left   = l;
	self->content_rect_width  = r - l;
	self->content_rect_height = b - t;
	gears_overlay_contentRect(self->overlay, t, l, b, r);
}
