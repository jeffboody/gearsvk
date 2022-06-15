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
* private                                                  *
***********************************************************/

int
clickTransition(vkui_widget_t* widget,
                int state, float x, float y)
{
	ASSERT(widget);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		vkui_screen_t* screen = widget->screen;

		vkui_window_t** _window;
		_window = (vkui_window_t**)
		          vkui_widget_widgetFnArg(widget);

		vkui_screen_windowPush(screen, *_window);
	}
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_layerHud_t*
gears_layerHud_new(struct gears_overlay_s* overlay)
{
	ASSERT(overlay);

	vkui_screen_t* screen = overlay->screen;

	vkui_windowInfo_t info =
	{
		.flags = VKUI_WINDOW_FLAG_LAYER1 |
		         VKUI_WINDOW_FLAG_TRANSPARENT
	};

	gears_layerHud_t* self;
	self = (gears_layerHud_t*)
	       vkui_window_new(screen,
	                       sizeof(gears_layerHud_t),
	                       &info);
	if(self == NULL)
	{
		return NULL;
	}

	const char* sprite_about[] =
	{
		"icons/ic_info_outline_white_24dp.png",
		NULL
	};

	vkui_widgetFn_t about_fn =
	{
		.arg      = (void*) &overlay->view_about,
		.click_fn = clickTransition
	};
	self->bulletbox_about = vkui_bulletbox_newPageItem(screen,
	                                                   &about_fn,
	                                                   sprite_about);
	if(self->bulletbox_about == NULL)
	{
		goto fail_bulletbox_about;
	}
	vkui_bulletbox_label(self->bulletbox_about,
	                     "%s", "Gears VK");

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

	vkui_textFn_t text_fn_fps =
	{
		.fn =
		{
			.priv = NULL
		}
	};

	cc_vec4f_t clear =
	{
		.a = 0.0f
	};

	self->text_fps = vkui_text_new(screen, 0,
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

	vkui_window_t* window = (vkui_window_t*) self;
	vkui_layer_t*  layer1 = vkui_window_layer1(window);
	vkui_layer_add(layer1, (vkui_widget_t*) self->bulletbox_about);
	vkui_layer_add(layer1, (vkui_widget_t*) self->text_fps);

	// success
	return self;

	// failure
	fail_text_fps:
		vkui_bulletbox_delete(&self->bulletbox_about);
	fail_bulletbox_about:
		vkui_window_delete((vkui_window_t**) &self);
	return NULL;
}

void gears_layerHud_delete(gears_layerHud_t** _self)
{
	ASSERT(_self);

	gears_layerHud_t* self = *_self;
	if(self)
	{
		vkui_window_t* window = (vkui_window_t*) self;
		vkui_layer_t*  layer1 = vkui_window_layer1(window);
		vkui_layer_clear(layer1);
		vkui_text_delete(&self->text_fps);
		vkui_bulletbox_delete(&self->bulletbox_about);
		vkui_window_delete((vkui_window_t**) &self);
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

	vkui_text_label(self->text_fps, "%i fps", fps);
}
