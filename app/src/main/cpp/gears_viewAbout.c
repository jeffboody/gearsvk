/*
 * Copyright (c) 2018 Jeff Boody
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_overlay.h"
#include "gears_renderer.h"
#include "gears_viewAbout.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int clickGithub(vkui_widget_t* widget,
                       void* priv, int state,
                       float x, float y)
{
	assert(widget);
	assert(priv);
	LOGD("debug x=%f, y=%f", x, y);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		gears_overlay_t*  overlay  = (gears_overlay_t*) priv;
		gears_renderer_t* renderer = overlay->renderer;
		gears_renderer_loadURL(renderer, "https://github.com/jeffboody/gearsvk/");
	}
	return 1;
}

static int clickIcons(vkui_widget_t* widget,
                      void* priv, int state,
                      float x, float y)
{
	assert(widget);
	assert(priv);
	LOGD("debug x=%f, y=%f", x, y);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		gears_overlay_t*  overlay  = (gears_overlay_t*) priv;
		gears_renderer_t* renderer = overlay->renderer;
		gears_renderer_loadURL(renderer, "https://github.com/google/material-design-icons/");
	}
	return 1;
}

static int clickBarlow(vkui_widget_t* widget,
                       void* priv, int state,
                       float x, float y)
{
	assert(widget);
	assert(priv);
	LOGD("debug x=%f, y=%f", x, y);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		gears_overlay_t*  overlay  = (gears_overlay_t*) priv;
		gears_renderer_t* renderer = overlay->renderer;
		gears_renderer_loadURL(renderer, "https://github.com/jpt/barlow/");
	}
	return 1;
}

static int clickExpat(vkui_widget_t* widget,
                      void* priv, int state,
                      float x, float y)
{
	assert(widget);
	assert(priv);
	LOGD("debug x=%f, y=%f", x, y);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		gears_overlay_t*  overlay  = (gears_overlay_t*) priv;
		gears_renderer_t* renderer = overlay->renderer;
		gears_renderer_loadURL(renderer, "https://github.com/jeffboody/libexpat/");
	}
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_viewAbout_t* gears_viewAbout_new(struct gears_overlay_s* overlay,
                                       vkui_widget_clickFn clickBack)
{
	assert(overlay);
	assert(clickBack);

	vkui_widgetLayout_t layout_listbox =
	{
		.border     = VKUI_WIDGET_BORDER_LARGE,
		.wrapx      = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchx   = 1.0f,
	};

	vkui_widgetScroll_t scroll_listbox =
	{
		.scroll_bar = 1,
		.color0 =
		{
			// ltgray
			.r = 0.95f,
			.g = 0.95f,
			.b = 0.95f,
			.a = 1.0f
		},
		.color1 =
		{
			// dkgray
			.r = 0.4f,
			.g = 0.4f,
			.b = 0.4f,
			.a = 1.0f
		}
	};

	vkui_widgetFn_t fn;
	memset(&fn, 0, sizeof(vkui_widgetFn_t));

	cc_vec4f_t clear =
	{
		.a = 0.0f
	};

	vkui_listbox_t* listbox;
	listbox = vkui_listbox_new(overlay->screen, 0,
	                           &layout_listbox,
	                           &scroll_listbox, &fn,
	                           VKUI_LISTBOX_ORIENTATION_VERTICAL,
	                           &clear);
	if(listbox == NULL)
	{
		return NULL;
	}

	vkui_widgetLayout_t layout_about =
	{
		.wrapy    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchy = 1.0f
	};

	vkui_widgetFn_t fn_about =
	{
		.priv     = (void*) overlay,
		.click_fn = clickBack
	};

	vkui_viewboxStyle_t viewbox_style =
	{
		.color_header =
		{
			.r = 0.04f,
			.g = 0.54f,
			.b = 0.89f,
			.a = 1.0f
		},
		.color_body =
		{
			.r = 1.0f,
			.g = 1.0f,
			.b = 1.0f,
			.a = 1.0f
		},
		.color_footer =
		{
			.r = 0.95f,
			.g = 0.95f,
			.b = 0.95f,
			.a = 1.0f
		},
		.bulletbox_style =
		{
			.color_icon =
			{
				.r = 1.0f,
				.g = 1.0f,
				.b = 1.0f,
				.a = 1.0f,
			},
			.text_style =
			{
				.font_type = VKUI_TEXT_FONTTYPE_BOLD,
				.size      = VKUI_TEXT_SIZE_MEDIUM,
				.spacing   = VKUI_TEXT_SPACING_MEDIUM,
				.color     =
				{
					.r = 1.0f,
					.g = 1.0f,
					.b = 1.0f,
					.a = 1.0f,
				}
			}
		}
	};

	const char* sprite_about[] =
	{
		"icons/ic_arrow_back_white_24dp.texz",
		NULL
	};

	gears_viewAbout_t* self;
	self = (gears_viewAbout_t*)
	       vkui_viewbox_new(overlay->screen,
	                       sizeof(gears_viewAbout_t),
	                       &layout_about, &fn_about,
	                       &viewbox_style, sprite_about,
	                       (vkui_widget_t*) listbox, NULL);
	if(self == NULL)
	{
		LOGE("fail_view");
		goto fail_view;
	}
	self->listbox = listbox;

	vkui_textStyle_t text_style_heading =
	{
		.font_type = VKUI_TEXT_FONTTYPE_BOLD,
		.size      = VKUI_TEXT_SIZE_MEDIUM,
		.spacing   = VKUI_TEXT_SPACING_MEDIUM,
		.color     =
		{
			.r = 0.0f,
			.g = 0.0f,
			.b = 0.0f,
			.a = 1.0f,
		}
	};

	vkui_textLayout_t text_layout =
	{
		.border = VKUI_WIDGET_BORDER_NONE
	};

	vkui_textFn_t text_fn_heading;
	memset(&text_fn_heading, 0, sizeof(vkui_textFn_t));

	vkui_text_t* text_intro;
	text_intro = vkui_text_new(overlay->screen, 0,
	                           &text_layout,
	                           &text_style_heading,
	                           &text_fn_heading,
	                           &clear);
	if(text_intro == NULL)
	{
		LOGE("fail_text_intro");
		goto fail_text_intro;
	}
	self->text_intro = text_intro;

	vkui_text_t* text_icons;
	text_icons = vkui_text_new(overlay->screen, 0,
	                           &text_layout,
	                           &text_style_heading,
	                           &text_fn_heading,
	                           &clear);
	if(text_icons == NULL)
	{
		LOGE("fail_text_icons");
		goto fail_text_icons;
	}
	self->text_icons = text_icons;

	vkui_text_t* text_barlow;
	text_barlow = vkui_text_new(overlay->screen, 0,
	                            &text_layout,
	                            &text_style_heading,
	                            &text_fn_heading,
	                            &clear);
	if(text_barlow == NULL)
	{
		LOGE("fail_text_barlow");
		goto fail_text_barlow;
	}
	self->text_barlow = text_barlow;

	vkui_text_t* text_expat;
	text_expat = vkui_text_new(overlay->screen, 0,
	                           &text_layout,
	                           &text_style_heading,
	                           &text_fn_heading,
	                           &clear);
	if(text_expat == NULL)
	{
		LOGE("fail_text_expat");
		goto fail_text_expat;
	}
	self->text_expat = text_expat;

	vkui_text_t* text_license;
	text_license = vkui_text_new(overlay->screen, 0,
	                             &text_layout,
	                             &text_style_heading,
	                             &text_fn_heading,
	                             &clear);
	if(text_license == NULL)
	{
		LOGE("fail_text_license");
		goto fail_text_license;
	}
	self->text_license = text_license;

	vkui_widgetLayout_t layout_textbox =
	{
		.wrapx    = VKUI_WIDGET_WRAP_STRETCH_PARENT,
		.stretchx = 1.0f,
	};

	vkui_textStyle_t style_textbox =
	{
		.font_type = VKUI_TEXT_FONTTYPE_REGULAR,
		.size      = VKUI_TEXT_SIZE_MEDIUM,
		.spacing   = VKUI_TEXT_SPACING_SMALL,
		.color     =
		{
			.r = 0.0f,
			.g = 0.0f,
			.b = 0.0f,
			.a = 1.0f,
		}
	};

	vkui_widgetScroll_t scroll_textbox;
	memset(&scroll_textbox, 0, sizeof(vkui_widgetScroll_t));

	vkui_widgetFn_t fn_textbox;
	memset(&fn_textbox, 0, sizeof(vkui_widgetFn_t));

	vkui_textbox_t* textbox_intro;
	textbox_intro = vkui_textbox_new(overlay->screen, 0,
	                                 &layout_textbox,
	                                 &scroll_textbox,
	                                 &fn_textbox,
	                                 &style_textbox);
	if(textbox_intro == NULL)
	{
		LOGE("fail_textbox_intro");
		goto fail_textbox_intro;
	}
	self->textbox_intro = textbox_intro;

	vkui_textbox_t* textbox_icons;
	textbox_icons = vkui_textbox_new(overlay->screen, 0,
	                                 &layout_textbox,
	                                 &scroll_textbox,
	                                 &fn_textbox,
	                                 &style_textbox);
	if(textbox_icons == NULL)
	{
		LOGE("fail_textbox_icons");
		goto fail_textbox_icons;
	}
	self->textbox_icons = textbox_icons;

	vkui_textbox_t* textbox_barlow;
	textbox_barlow = vkui_textbox_new(overlay->screen, 0,
	                                  &layout_textbox,
	                                  &scroll_textbox,
	                                  &fn_textbox,
	                                  &style_textbox);
	if(textbox_barlow == NULL)
	{
		LOGE("fail_textbox_barlow");
		goto fail_textbox_barlow;
	}
	self->textbox_barlow = textbox_barlow;

	vkui_textbox_t* textbox_expat;
	textbox_expat = vkui_textbox_new(overlay->screen, 0,
	                                 &layout_textbox,
	                                 &scroll_textbox,
	                                 &fn_textbox,
	                                 &style_textbox);
	if(textbox_expat == NULL)
	{
		LOGE("fail_textbox_expat");
		goto fail_textbox_expat;
	}
	self->textbox_expat = textbox_expat;

	vkui_textbox_t* textbox_license;
	textbox_license = vkui_textbox_new(overlay->screen, 0,
	                                   &layout_textbox,
	                                   &scroll_textbox,
	                                   &fn_textbox,
	                                   &style_textbox);
	if(textbox_license == NULL)
	{
		LOGE("fail_textbox_license");
		goto fail_textbox_license;
	}
	self->textbox_license = textbox_license;

	vkui_textStyle_t style_linkbox =
	{
		.font_type = VKUI_TEXT_FONTTYPE_REGULAR,
		.size      = VKUI_TEXT_SIZE_MEDIUM,
		.spacing   = VKUI_TEXT_SPACING_SMALL,
		.color     =
		{
			.r = 0.04f,
			.g = 0.54f,
			.b = 0.89f,
			.a = 1.0f
		}
	};

	vkui_widgetFn_t fn_link =
	{
		.priv     = (void*) overlay,
		.click_fn = clickGithub
	};

	vkui_textbox_t* linkbox_github;
	linkbox_github = vkui_textbox_new(overlay->screen, 0,
	                                  &layout_textbox,
	                                  &scroll_textbox,
	                                  &fn_link,
	                                  &style_linkbox);
	if(linkbox_github == NULL)
	{
		LOGE("fail_linkbox_github");
		goto fail_linkbox_github;
	}
	self->linkbox_github = linkbox_github;

	vkui_textbox_t* linkbox_icons;
	fn_link.click_fn = clickIcons;
	linkbox_icons = vkui_textbox_new(overlay->screen, 0,
	                                 &layout_textbox,
	                                 &scroll_textbox,
	                                 &fn_link,
	                                 &style_linkbox);
	if(linkbox_icons == NULL)
	{
		LOGE("fail_linkbox_icons");
		goto fail_linkbox_icons;
	}
	self->linkbox_icons = linkbox_icons;

	vkui_textbox_t* linkbox_barlow;
	fn_link.click_fn = clickBarlow;
	linkbox_barlow = vkui_textbox_new(overlay->screen, 0,
	                                  &layout_textbox,
	                                  &scroll_textbox,
	                                  &fn_link,
	                                  &style_linkbox);
	if(linkbox_barlow == NULL)
	{
		LOGE("fail_linkbox_barlow");
		goto fail_linkbox_barlow;
	}
	self->linkbox_barlow = linkbox_barlow;

	vkui_textbox_t* linkbox_expat;
	fn_link.click_fn = clickExpat;
	linkbox_expat = vkui_textbox_new(overlay->screen, 0,
	                                 &layout_textbox,
	                                 &scroll_textbox,
	                                 &fn_link,
	                                 &style_linkbox);
	if(linkbox_expat == NULL)
	{
		LOGE("fail_linkbox_expat");
		goto fail_linkbox_expat;
	}
	self->linkbox_expat = linkbox_expat;

	vkui_viewbox_label(&self->viewbox, "%s", "About");

	vkui_text_label(text_intro, "%s", "Introduction");
	vkui_textbox_printf(textbox_intro, "%s", "Gears for Android is a heavily modified port of the famous \"gears\" demo");
	vkui_textbox_printf(textbox_intro, "%s", "to Android.");
	vkui_textbox_printf(textbox_intro, "%s", "");
	vkui_textbox_printf(textbox_intro, "%s", "The Gears demo is an open source project intended to help developers learn");
	vkui_textbox_printf(textbox_intro, "%s", "how to create OpenGL ES programs on Android. The Gears demo was originally");
	vkui_textbox_printf(textbox_intro, "%s", "written by Brian Paul as part of the Mesa3D project. My implementation");
	vkui_textbox_printf(textbox_intro, "%s", "includes variations for Java/OpenGL ES 1.x, Java/C/OpenGL ES 1.x,");
	vkui_textbox_printf(textbox_intro, "%s", "Java/C/OpenGL ES 2.0 and C/Vulkan. The Vulkan edition includes two versions.");
	vkui_textbox_printf(textbox_intro, "%s", "The first version (the master branch) is implemented using the Vulkan library");
	vkui_textbox_printf(textbox_intro, "%s", "directly while the second version (the master-vkk branch) is implemented using");
	vkui_textbox_printf(textbox_intro, "%s", "a Vulkan wrapper library called VKK (The Vulkan Kit). Going forward only the");
	vkui_textbox_printf(textbox_intro, "%s", "VKK version will be maintained.");
	vkui_textbox_printf(textbox_intro, "%s", "");
	vkui_textbox_printf(textbox_intro, "%s", "The FPS (frames-per-second) counter is often used as a benchmark metric for");
	vkui_textbox_printf(textbox_intro, "%s", "graphics programs. On Android the frame rate is limited by v-sync (typically");
	vkui_textbox_printf(textbox_intro, "%s", "60 FPS) which is the fastest rate that a display can refresh the screen.");
	vkui_textbox_printf(textbox_intro, "%s", "Since Gears is capable of rendering much faster than v-sync on most devices");
	vkui_textbox_printf(textbox_intro, "%s", "it provides limited benchmarking value.");
	vkui_textbox_printf(textbox_intro, "%s", "");
	vkui_textbox_printf(textbox_intro, "%s", "Send questions or comments to Jeff Boody at jeffboody@gmail.com.");
	vkui_textbox_printf(linkbox_github, "%s", "https://github.com/jeffboody/gearsvk/");
	vkui_textbox_printf(linkbox_github, "%s", "");

	vkui_text_label(text_icons, "%s", "Material Design Icons");
	vkui_textbox_printf(textbox_icons, "%s", "By Google, under the Apache License, Version 2.0.");
	vkui_textbox_printf(linkbox_icons, "%s", "https://github.com/google/material-design-icons/");
	vkui_textbox_printf(linkbox_icons, "%s", "");

	vkui_text_label(text_barlow, "%s", "Barlow Font");
	vkui_textbox_printf(textbox_barlow, "%s", "Font support is provided by The Barlow Project Authors,");
	vkui_textbox_printf(textbox_barlow, "%s", "under the SIL Open Font License, Version 1.1.");
	vkui_textbox_printf(linkbox_barlow, "%s", "https://github.com/jpt/barlow/");
	vkui_textbox_printf(linkbox_barlow, "%s", "");

	vkui_text_label(text_expat, "%s", "Expat XML Parser");
	vkui_textbox_printf(textbox_expat, "%s", "XML parsing provided by the Expat XML Parser");
	vkui_textbox_printf(textbox_expat, "%s", "under the MIT license.");
	vkui_textbox_printf(linkbox_expat, "%s", "https://github.com/jeffboody/libexpat/");
	vkui_textbox_printf(linkbox_expat, "%s", "");

	vkui_text_label(text_license, "%s", "Gears Source License");
	vkui_textbox_printf(textbox_license, "%s", "Copyright (c) 2009-2011 Jeff Boody");
	vkui_textbox_printf(textbox_license, "%s", "");
	vkui_textbox_printf(textbox_license, "%s", "Gears for Android is a heavily modified port of the famous \"gears\" demo. ");
	vkui_textbox_printf(textbox_license, "%s", "As such, it is a derived work subject to the license requirements (below) ");
	vkui_textbox_printf(textbox_license, "%s", "of the original work.");
	vkui_textbox_printf(textbox_license, "%s", "");
	vkui_textbox_printf(textbox_license, "%s", "Copyright (c) 1999-2001 Brian Paul All Rights Reserved.");
	vkui_textbox_printf(textbox_license, "%s", "");
	vkui_textbox_printf(textbox_license, "%s", "Permission is hereby granted, free of charge, to any person obtaining a");
	vkui_textbox_printf(textbox_license, "%s", "copy of this software and associated documentation files (the \"Software\"),");
	vkui_textbox_printf(textbox_license, "%s", "to deal in the Software without restriction, including without limitation");
	vkui_textbox_printf(textbox_license, "%s", "the rights to use, copy, modify, merge, publish, distribute, sublicense,");
	vkui_textbox_printf(textbox_license, "%s", "and/or sell copies of the Software, and to permit persons to whom the");
	vkui_textbox_printf(textbox_license, "%s", "Software is furnished to do so, subject to the following conditions:");
	vkui_textbox_printf(textbox_license, "%s", "");
	vkui_textbox_printf(textbox_license, "%s", "The above copyright notice and this permission notice shall be included");
	vkui_textbox_printf(textbox_license, "%s", "in all copies or substantial portions of the Software.");
	vkui_textbox_printf(textbox_license, "%s", "");
	vkui_textbox_printf(textbox_license, "%s", "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
	vkui_textbox_printf(textbox_license, "%s", "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
	vkui_textbox_printf(textbox_license, "%s", "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE");
	vkui_textbox_printf(textbox_license, "%s", "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
	vkui_textbox_printf(textbox_license, "%s", "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
	vkui_textbox_printf(textbox_license, "%s", "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN");
	vkui_textbox_printf(textbox_license, "%s", "THE SOFTWARE.");

	vkui_listbox_add(listbox, (vkui_widget_t*) text_intro);
	vkui_listbox_add(listbox, (vkui_widget_t*) textbox_intro);
	vkui_listbox_add(listbox, (vkui_widget_t*) linkbox_github);
	vkui_listbox_add(listbox, (vkui_widget_t*) text_icons);
	vkui_listbox_add(listbox, (vkui_widget_t*) textbox_icons);
	vkui_listbox_add(listbox, (vkui_widget_t*) linkbox_icons);
	vkui_listbox_add(listbox, (vkui_widget_t*) text_barlow);
	vkui_listbox_add(listbox, (vkui_widget_t*) textbox_barlow);
	vkui_listbox_add(listbox, (vkui_widget_t*) linkbox_barlow);
	vkui_listbox_add(listbox, (vkui_widget_t*) text_expat);
	vkui_listbox_add(listbox, (vkui_widget_t*) textbox_expat);
	vkui_listbox_add(listbox, (vkui_widget_t*) linkbox_expat);
	vkui_listbox_add(listbox, (vkui_widget_t*) text_license);
	vkui_listbox_add(listbox, (vkui_widget_t*) textbox_license);

	// success
	return self;

	// failure
	fail_linkbox_expat:
		vkui_textbox_delete(&linkbox_barlow);
	fail_linkbox_barlow:
		vkui_textbox_delete(&linkbox_icons);
	fail_linkbox_icons:
		vkui_textbox_delete(&linkbox_github);
	fail_linkbox_github:
		vkui_textbox_delete(&textbox_license);
	fail_textbox_license:
		vkui_textbox_delete(&textbox_expat);
	fail_textbox_expat:
		vkui_textbox_delete(&textbox_barlow);
	fail_textbox_barlow:
		vkui_textbox_delete(&textbox_icons);
	fail_textbox_icons:
		vkui_textbox_delete(&textbox_intro);
	fail_textbox_intro:
		vkui_text_delete(&text_license);
	fail_text_license:
		vkui_text_delete(&text_expat);
	fail_text_expat:
		vkui_text_delete(&text_barlow);
	fail_text_barlow:
		vkui_text_delete(&text_icons);
	fail_text_icons:
		vkui_text_delete(&text_intro);
	fail_text_intro:
		vkui_viewbox_delete((vkui_viewbox_t**) &self);
	fail_view:
		vkui_listbox_delete(&listbox);
	return NULL;
}

void gears_viewAbout_delete(gears_viewAbout_t** _self)
{
	assert(_self);

	gears_viewAbout_t* self = *_self;
	if(self)
	{
		vkui_listbox_clear(self->listbox);
		vkui_textbox_delete(&self->linkbox_expat);
		vkui_textbox_delete(&self->linkbox_barlow);
		vkui_textbox_delete(&self->linkbox_icons);
		vkui_textbox_delete(&self->linkbox_github);
		vkui_textbox_delete(&self->textbox_license);
		vkui_textbox_delete(&self->textbox_expat);
		vkui_textbox_delete(&self->textbox_barlow);
		vkui_textbox_delete(&self->textbox_icons);
		vkui_textbox_delete(&self->textbox_intro);
		vkui_text_delete(&self->text_license);
		vkui_text_delete(&self->text_expat);
		vkui_text_delete(&self->text_barlow);
		vkui_text_delete(&self->text_icons);
		vkui_text_delete(&self->text_intro);
		vkui_listbox_delete(&self->listbox);
		vkui_viewbox_delete((vkui_viewbox_t**) _self);
	}
}
