/*
 * Copyright (c) 2009-2010 Jeff Boody
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

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_glsm.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void gears_glsm_draincache(gears_glsm_t* self)
{
	ASSERT(self);

	// empty the vb list
	cc_vec3f_t*    v;
	cc_listIter_t* iter = cc_list_head(self->cache_vb);
	while(iter)
	{
		v = (cc_vec3f_t*)
		    cc_list_remove(self->cache_vb, &iter);
		free(v);
	}

	// empty the nb list
	iter = cc_list_head(self->cache_nb);
	while(iter)
	{
		v = (cc_vec3f_t*)
		    cc_list_remove(self->cache_nb, &iter);
		free(v);
	}
}

static void gears_glsm_freebuffers(gears_glsm_t* self)
{
	ASSERT(self);

	free(self->vb);
	free(self->nb);
	self->vb = NULL;
	self->nb = NULL;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_glsm_t* gears_glsm_new(void)
{

	gears_glsm_t* self = (gears_glsm_t*) malloc(sizeof(gears_glsm_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->normal.x = 0.0f;
	self->normal.y = 0.0f;
	self->normal.z = 1.0f;
	self->ec       = 0;
	self->vb       = NULL;
	self->nb       = NULL;
	self->status   = GEARS_GLSM_INCOMPLETE;

	self->cache_vb = cc_list_new();
	if(self->cache_vb == NULL)
	{
		goto fail_cache_vb;
	}

	self->cache_nb = cc_list_new();
	if(self->cache_nb == NULL)
	{
		goto fail_cache_nb;
	}

	// success
	return self;

	// failure
	fail_cache_nb:
		cc_list_delete(&self->cache_vb);
	fail_cache_vb:
		free(self);
	return NULL;
}

void gears_glsm_delete(gears_glsm_t** _self)
{
	// *_self can be null
	ASSERT(_self);

	gears_glsm_t* self = *_self;
	if(self)
	{
		gears_glsm_draincache(self);
		cc_list_delete(&self->cache_vb);
		cc_list_delete(&self->cache_nb);
		gears_glsm_freebuffers(self);
		free(self);
		*_self = NULL;
	}
}

void gears_glsm_begin(gears_glsm_t* self)
{
	ASSERT(self);

	self->normal.x = 0.0f;
	self->normal.y = 0.0f;
	self->normal.z = 1.0f;
	self->ec       = 0;
	self->status   = GEARS_GLSM_INCOMPLETE;
	gears_glsm_draincache(self);
	gears_glsm_freebuffers(self);
}

void gears_glsm_normal3f(gears_glsm_t* self, float x, float y, float z)
{
	ASSERT(self);

	if(self->status != GEARS_GLSM_INCOMPLETE)
	{
		return;
	}

	self->normal.x = x;
	self->normal.y = y;
	self->normal.z = z;
}

void gears_glsm_vertex3f(gears_glsm_t* self, float x, float y, float z)
{
	ASSERT(self);

	if(self->status != GEARS_GLSM_INCOMPLETE)
	{
		return;
	}

	cc_vec3f_t* v = (cc_vec3f_t*) malloc(sizeof(cc_vec3f_t));
	if(v == NULL)
	{
		LOGE("malloc failed");
		goto fail_malloc_v;
	}
	v->x = x;
	v->y = y;
	v->z = z;

	if(cc_list_append(self->cache_vb, NULL,
	                  (const void*) v) == NULL)
	{
		goto fail_append_vb;
	}

	cc_vec3f_t* n = (cc_vec3f_t*) malloc(sizeof(cc_vec3f_t));
	if(n == NULL)
	{
		LOGE("malloc failed");
		goto fail_malloc_n;
	}
	n->x = self->normal.x;
	n->y = self->normal.y;
	n->z = self->normal.z;

	if(cc_list_append(self->cache_nb, NULL,
	                  (const void*) n) == NULL)
	{
		goto fail_append_nb;
	}

	// success
	return;

	// failure
	fail_append_nb:
		free(n);
	fail_malloc_n:
		// cache drained below
	fail_append_vb:
		free(v);
	fail_malloc_v:
		gears_glsm_draincache(self);
		self->status = GEARS_GLSM_ERROR;
}

void gears_glsm_end(gears_glsm_t* self)
{
	ASSERT(self);

	if(self->status != GEARS_GLSM_INCOMPLETE)
	{
		return;
	}

	// initialize buffers
	self->ec = cc_list_size(self->cache_vb);   // vertex count
	self->vb = (float*) malloc(3 * self->ec * sizeof(float));
	self->nb = (float*) malloc(3 * self->ec * sizeof(float));
	if((self->vb == NULL) || (self->nb == NULL))
	{
		LOGE("malloc failed");
		goto fail_malloc;
	}

	// replay the cached vertices and normals
	uint32_t vi;
	cc_vec3f_t* v;
	cc_vec3f_t* n;
	cc_listIter_t* iter_vb = cc_list_head(self->cache_vb);
	cc_listIter_t* iter_nb = cc_list_head(self->cache_nb);
	for(vi = 0; vi < self->ec; ++vi)
	{
		if((iter_vb == NULL) || (iter_nb == NULL))
		{
			goto fail_vert;
		}

		v = (cc_vec3f_t*) cc_list_remove(self->cache_vb,
		                                 &iter_vb);
		if(v == NULL)
		{
			goto fail_vert;
		}

		n = (cc_vec3f_t*) cc_list_remove(self->cache_nb,
		                                 &iter_nb);
		if(n == NULL)
		{
			goto fail_norm;
		}

		self->vb[3 * vi    ] = v->x;
		self->vb[3 * vi + 1] = v->y;
		self->vb[3 * vi + 2] = v->z;
		self->nb[3 * vi    ] = n->x;
		self->nb[3 * vi + 1] = n->y;
		self->nb[3 * vi + 2] = n->z;

		// free the cache as we go
		free(v);
		free(n);
	}

	// success
	self->status = GEARS_GLSM_COMPLETE;
	return;

	// failure
	fail_norm:
		free(v);
	fail_vert:
		gears_glsm_freebuffers(self);
	fail_malloc:
		gears_glsm_draincache(self);
	self->status = GEARS_GLSM_ERROR;
}

int gears_glsm_status(gears_glsm_t* self)
{
	ASSERT(self);
	return self->status;
}
