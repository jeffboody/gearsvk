/*
 * Copyright (c) 2018 Jeff Boody
 */

#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_layerHud.h"
#include "gears_overlay.h"
#include "gears_renderer.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int
gears_layerHud_refresh(vkk_uiWidget_t* widget)
{
	ASSERT(widget);

	gears_layerHud_t* self = (gears_layerHud_t*) widget;

	gears_overlay_t* overlay;
	overlay = (gears_overlay_t*) vkk_uiWidget_priv(widget);

	vkk_engine_t* engine = overlay->engine;

	vkk_renderer_t* rend;
	rend = vkk_engine_defaultRenderer(engine);

	int fps = vkk_renderer_fps(rend);
	if(fps != self->last_fps)
	{
		vkk_uiText_label(self->text_fps, "%i fps", fps);
		self->last_fps = fps;
	}

	return 1;
}

static vkk_uiWidget_t*
gears_layerHud_rendererAction(vkk_uiWidget_t* widget,
                              vkk_uiWidgetActionInfo_t* info)
{
	ASSERT(widget);
	ASSERT(info);

	// widget is gb_renderer

	gears_overlay_t* overlay;
	overlay = (gears_overlay_t*) vkk_uiWidget_priv(widget);

	if(info->action == VKK_UI_WIDGET_ACTION_DOWN)
	{
		return widget;
	}
	else if(info->action == VKK_UI_WIDGET_ACTION_DRAG)
	{
		// accept 1 or 2 finger drags
		gears_renderer_rotate(overlay->renderer,
		                      info->drag.x, info->drag.y);
		return widget;
	}
	else if(info->action == VKK_UI_WIDGET_ACTION_SCALE)
	{
		gears_renderer_scale(overlay->renderer, info->scale);
		return widget;
	}

	return NULL;
}

static void
gears_layerHud_rendererDraw(vkk_uiWidget_t* widget)
{
	ASSERT(widget);

	// widget is gb_renderer

	gears_overlay_t* overlay;
	overlay = (gears_overlay_t*) vkk_uiWidget_priv(widget);

	cc_rect1f_t* rect_draw = vkk_uiWidget_rectDraw(widget);

	gears_renderer_draw(overlay->renderer,
	                    (float) rect_draw->w,
	                    (float) rect_draw->h);
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_layerHud_t*
gears_layerHud_new(gears_overlay_t* overlay)
{
	ASSERT(overlay);

	vkk_uiScreen_t* screen = overlay->screen;

	vkk_uiWindowFn_t wfn =
	{
		.priv       = overlay,
		.refresh_fn = gears_layerHud_refresh,
	};

	uint32_t flags = VKK_UI_WINDOW_FLAG_LAYER0 |
	                 VKK_UI_WINDOW_FLAG_LAYER1 |
	                 VKK_UI_WINDOW_FLAG_TRANSPARENT;

	gears_layerHud_t* self;
	self = (gears_layerHud_t*)
	       vkk_uiWindow_new(screen, sizeof(gears_layerHud_t),
	                        &wfn, flags);
	if(self == NULL)
	{
		return NULL;
	}

	const char* sprite_about[] =
	{
		"icons/ic_info_outline_white_24dp.png",
		NULL
	};

	vkk_uiBulletBoxFn_t about_fn =
	{
		.priv     = &overlay->view_about,
		.click_fn = vkk_uiWidget_clickTransition,
	};
	self->bulletbox_about = vkk_uiBulletBox_newPageItem(screen,
	                                                    &about_fn,
	                                                    sprite_about);
	if(self->bulletbox_about == NULL)
	{
		goto fail_bulletbox_about;
	}
	vkk_uiBulletBox_label(self->bulletbox_about,
	                      "%s", "Gears VK");

	vkk_uiTextLayout_t text_layout =
	{
		.anchor = VKK_UI_WIDGET_ANCHOR_BR
	};

	vkk_uiTextStyle_t text_style_fps =
	{
		.font_type = VKK_UI_TEXT_FONTTYPE_REGULAR,
		.size      = VKK_UI_TEXT_SIZE_SMALL,
		.color     =
		{
			.r = 1.0f,
			.g = 1.0f,
			.b = 0.235f,
			.a = 1.0f
		}
	};

	vkk_uiTextFn_t text_fn_fps =
	{
		.priv = NULL
	};

	cc_vec4f_t clear =
	{
		.a = 0.0f
	};

	self->text_fps = vkk_uiText_new(screen, 0,
	                                &text_fn_fps,
	                                &text_layout,
	                                &text_style_fps,
	                                &clear);
	if(self->text_fps == NULL)
	{
		goto fail_text_fps;
	}
	vkk_uiText_label(self->text_fps, "%s", "0 fps");

	vkk_uiGraphicsBoxFn_t gbfn =
	{
		.priv      = overlay,
		.action_fn = gears_layerHud_rendererAction,
		.draw_fn   = gears_layerHud_rendererDraw,
	};

	vkk_uiWidgetLayout_t gb_layout =
	{
		.wrapx    = VKK_UI_WIDGET_WRAP_STRETCH_PARENT,
		.wrapy    = VKK_UI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchx = 1.0f,
		.stretchy = 1.0f,
	};

	self->gb_renderer = vkk_uiGraphicsBox_new(screen, 0,
	                                          &gbfn,
	                                          &gb_layout,
	                                          1, &clear);
	if(self->gb_renderer == NULL)
	{
		goto fail_gb_renderer;
	}

	vkk_uiWindow_t* window = (vkk_uiWindow_t*) self;
	vkk_uiLayer_t*  layer0 = vkk_uiWindow_layer0(window);
	vkk_uiLayer_t*  layer1 = vkk_uiWindow_layer1(window);
	vkk_uiLayer_add(layer0, (vkk_uiWidget_t*) self->gb_renderer);
	vkk_uiLayer_add(layer1, (vkk_uiWidget_t*) self->bulletbox_about);
	vkk_uiLayer_add(layer1, (vkk_uiWidget_t*) self->text_fps);

	// success
	return self;

	// failure
	fail_gb_renderer:
		vkk_uiText_delete(&self->text_fps);
	fail_text_fps:
		vkk_uiBulletBox_delete(&self->bulletbox_about);
	fail_bulletbox_about:
		vkk_uiWindow_delete((vkk_uiWindow_t**) &self);
	return NULL;
}

void gears_layerHud_delete(gears_layerHud_t** _self)
{
	ASSERT(_self);

	gears_layerHud_t* self = *_self;
	if(self)
	{
		vkk_uiWindow_t* window = (vkk_uiWindow_t*) self;
		vkk_uiLayer_t*  layer0 = vkk_uiWindow_layer0(window);
		vkk_uiLayer_t*  layer1 = vkk_uiWindow_layer1(window);
		vkk_uiLayer_clear(layer0);
		vkk_uiLayer_clear(layer1);
		vkk_uiGraphicsBox_delete(&self->gb_renderer);
		vkk_uiText_delete(&self->text_fps);
		vkk_uiBulletBox_delete(&self->bulletbox_about);
		vkk_uiWindow_delete((vkk_uiWindow_t**) &self);
	}
}
