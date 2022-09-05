/*
 * Copyright (c) 2018 Jeff Boody
 */

#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_layerHud.h"
#include "gears_overlay.h"

/***********************************************************
* public                                                   *
***********************************************************/

gears_layerHud_t*
gears_layerHud_new(struct gears_overlay_s* overlay)
{
	ASSERT(overlay);

	vkk_uiScreen_t* screen = overlay->screen;

	vkk_uiWindowFn_t wfn =
	{
		.priv = NULL
	};

	uint32_t flags = VKK_UI_WINDOW_FLAG_LAYER1 |
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
	self->fps = 0;

	vkk_uiWindow_t* window = (vkk_uiWindow_t*) self;
	vkk_uiLayer_t*  layer1 = vkk_uiWindow_layer1(window);
	vkk_uiLayer_add(layer1, (vkk_uiWidget_t*) self->bulletbox_about);
	vkk_uiLayer_add(layer1, (vkk_uiWidget_t*) self->text_fps);

	// success
	return self;

	// failure
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
		vkk_uiLayer_t*  layer1 = vkk_uiWindow_layer1(window);
		vkk_uiLayer_clear(layer1);
		vkk_uiText_delete(&self->text_fps);
		vkk_uiBulletBox_delete(&self->bulletbox_about);
		vkk_uiWindow_delete((vkk_uiWindow_t**) &self);
	}
}

void gears_layerHud_updateFps(gears_layerHud_t* self, int fps)
{
	ASSERT(self);

	if(self->fps == fps)
	{
		return;
	}
	self->fps = fps;

	vkk_uiText_label(self->text_fps, "%i fps", fps);
}
