/*
 * Copyright (c) 2018 Jeff Boody
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_layerHud.h"
#include "gears_overlay.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int clickAbout(vkui_widget_t* widget,
                      void* priv, int state,
                      float x, float y)
{
	assert(widget);
	assert(priv);

	gears_overlay_t* overlay = (gears_overlay_t*) priv;
	if(state == VKUI_WIDGET_POINTER_UP)
	{
		overlay->draw_mode = GEARS_OVERLAY_DRAWMODE_ABOUT;
		vkui_layer_clear(overlay->layer_show);
		vkui_layer_add(overlay->layer_show,
		               (vkui_widget_t*) overlay->view_about);
	}
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_layerHud_t* gears_layerHud_new(struct gears_overlay_s* overlay)
{
	assert(overlay);

	cc_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	vkui_widgetLayout_t layout_hud =
	{
		.border   = VKUI_WIDGET_BORDER_LARGE,
		.wrapx    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.wrapy    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchx = 1.0f,
		.stretchy = 1.0f
	};

	gears_layerHud_t* self;
	self = (gears_layerHud_t*)
	       vkui_layer_new(overlay->screen,
	                      sizeof(gears_layerHud_t),
	                      &layout_hud, &clear);
	if(self == NULL)
	{
		return NULL;
	}

	vkui_bulletboxStyle_t style_about =
	{
		.color_icon =
		{
			.r = 1.0f,
			.g = 1.0f,
			.b = 1.0f,
			.a = 1.0f
		},
		.text_style =
		{
			.font_type = VKUI_TEXT_FONTTYPE_REGULAR,
			.size      = VKUI_TEXT_SIZE_MEDIUM,
			.spacing   = VKUI_TEXT_SPACING_MEDIUM,
			.color     =
			{
				.r = 1.0f,
				.g = 1.0f,
				.b = 1.0f,
				.a = 1.0f
			}
		}
	};

	vkui_widgetFn_t fn =
	{
		.priv     = (void*) overlay,
		.click_fn = clickAbout,
	};

	const char* sprite_about[] =
	{
		"ic_info_outline_white_24dp.texz",
		NULL
	};

	self->bulletbox_about = vkui_bulletbox_new(overlay->screen,
	                                           0,
	                                           VKUI_WIDGET_ANCHOR_TL,
	                                           &fn,
	                                           &style_about,
	                                           sprite_about);
	if(self->bulletbox_about == NULL)
	{
		goto fail_bulletbox_about;
	}
	vkui_bulletbox_label(self->bulletbox_about,
	                     "%s", "Gears");

	vkui_textLayout_t text_layout =
	{
		.anchor = VKUI_WIDGET_ANCHOR_BR
	};

	vkui_textStyle_t text_style_fps =
	{
		.font_type = VKUI_TEXT_FONTTYPE_REGULAR,
		.size      = VKUI_TEXT_SIZE_SMALL,
		.color     =
		{
			.r = 1.0f,
			.g = 1.0f,
			.b = 0.235f,
			.a = 1.0f
		}
	};

	vkui_textFn_t text_fn_fps;
	memset(&text_fn_fps, 0, sizeof(vkui_textFn_t));

	self->text_fps = vkui_text_new(overlay->screen, 0,
	                               &text_layout,
	                               &text_style_fps,
	                               &text_fn_fps,
	                               &clear);
	if(self->text_fps == NULL)
	{
		goto fail_text_fps;
	}
	vkui_text_label(self->text_fps, "%s", "0 fps");
	self->fps = 0;

	vkui_layer_t* layer = (vkui_layer_t*) self;
	vkui_layer_add(layer,
	               (vkui_widget_t*) self->bulletbox_about);
	vkui_layer_add(layer, (vkui_widget_t*) self->text_fps);

	// success
	return self;

	// failure
	fail_text_fps:
		vkui_bulletbox_delete(&self->bulletbox_about);
	fail_bulletbox_about:
		vkui_layer_delete((vkui_layer_t**) &self);
	return NULL;
}

void gears_layerHud_delete(gears_layerHud_t** _self)
{
	assert(_self);

	gears_layerHud_t* self = *_self;
	if(self)
	{
		vkui_layer_clear((vkui_layer_t*) self);
		vkui_text_delete(&self->text_fps);
		vkui_bulletbox_delete(&self->bulletbox_about);
		vkui_layer_delete((vkui_layer_t**) _self);
	}
}

void gears_layerHud_updateFps(gears_layerHud_t* self, int fps)
{
	assert(self);

	if(self->fps == fps)
	{
		return;
	}
	self->fps = fps;

	vkui_text_label(self->text_fps, "%i fps", fps);
}
