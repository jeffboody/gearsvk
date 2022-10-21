/*
 * Copyright (c) 2018 Jeff Boody
 */

#include <stdlib.h>
#include <string.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "gears_overlay.h"
#include "gears_renderer.h"
#include "gears_viewAbout.h"

const char* GEARS_URL = "https://github.com/jeffboody/gearsvk/";

/***********************************************************
* private                                                  *
***********************************************************/

static vkk_uiWidget_t*
gears_viewAbout_rendererAction(vkk_uiWidget_t* widget,
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
gears_viewAbout_rendererDraw(vkk_uiWidget_t* widget)
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

gears_viewAbout_t*
gears_viewAbout_new(gears_overlay_t* overlay)
{
	ASSERT(overlay);

	vkk_uiScreen_t* screen = overlay->screen;

	vkk_uiWindowFn_t wfn =
	{
		.priv = NULL
	};

	uint32_t flags = VKK_UI_WINDOW_FLAG_TITLE |
		             VKK_UI_WINDOW_FLAG_PAGE_DEFAULT;

	gears_viewAbout_t* self;
	self = (gears_viewAbout_t*)
	       vkk_uiWindow_new(screen,
	                        sizeof(gears_viewAbout_t),
	                        &wfn, flags);
	if(self == NULL)
	{
		return NULL;
	}

	vkk_uiGraphicsBoxFn_t gbfn =
	{
		.priv      = overlay,
		.action_fn = gears_viewAbout_rendererAction,
		.draw_fn   = gears_viewAbout_rendererDraw,
	};

	vkk_uiWidgetLayout_t gb_layout =
	{
		.border   = VKK_UI_WIDGET_BORDER_LARGE,
		.anchor   = VKK_UI_WIDGET_ANCHOR_TC,
		.wrapx    = VKK_UI_WIDGET_WRAP_STRETCH_TEXT_VLARGE,
		.wrapy    = VKK_UI_WIDGET_WRAP_STRETCH_TEXT_VLARGE,
		.stretchx = 5.5f,
		.stretchy = 5.5f
	};

	cc_vec4f_t color = { .a=1.0f };
	self->gb_renderer = vkk_uiGraphicsBox_new(screen, 0,
	                                          &gbfn,
	                                          &gb_layout,
	                                          1, &color);
	if(self->gb_renderer == NULL)
	{
		goto fail_gb_renderer;
	}

	self->text_intro = vkk_uiText_newPageHeading(screen);
	if(self->text_intro == NULL)
	{
		goto fail_text_intro;
	}

	self->text_license = vkk_uiText_newPageHeading(screen);
	if(self->text_license == NULL)
	{
		goto fail_text_license;
	}

	self->textbox_intro = vkk_uiTextBox_newPageParagraph(screen);
	if(self->textbox_intro == NULL)
	{
		goto fail_textbox_intro;
	}

	self->textbox_license = vkk_uiTextBox_newPageParagraph(screen);
	if(self->textbox_license == NULL)
	{
		goto fail_textbox_license;
	}

	vkk_uiTextBoxFn_t linkbox_github_fn =
	{
		.priv     = (void*) GEARS_URL,
		.click_fn = vkk_uiWidget_clickUrl,
	};

	self->linkbox_github = vkk_uiTextBox_newPageButton(screen,
	                                                   &linkbox_github_fn);
	if(self->linkbox_github == NULL)
	{
		goto fail_linkbox_github;
	}

	vkk_uiWindow_t* window = (vkk_uiWindow_t*) self;
	vkk_uiWindow_label(window, "%s", "About");
	vkk_uiText_label(self->text_intro, "%s", "About");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "Gears VK is a heavily modified port of the famous \"gears\" demo.");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "The Gears demo is an open source project intended to help developers to");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "learn graphics APIs. The Gears demo was originally");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "written by Brian Paul as part of the Mesa3D project.");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "I've ported this app between multiple languages and graphics APIs over the");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "years and have found it a very useful learning tool.");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "");
	vkk_uiTextBox_printf(self->textbox_intro, "%s", "Source code is available on Github.");
	vkk_uiTextBox_printf(self->linkbox_github, "%s", "https://github.com/jeffboody/gearsvk/");
	vkk_uiTextBox_printf(self->linkbox_github, "%s", "");

	vkk_uiText_label(self->text_license, "%s", "License");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "This port of Gears VK was implemented by");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Jeff Boody (jeffboody@gmail.com) under The MIT License.");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Copyright (c) 2009-2011 Jeff Boody");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Gears for Android is a heavily modified port of the famous \"gears\" demo. ");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "As such, it is a derived work subject to the license requirements (below) ");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "of the original work.");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Copyright (c) 1999-2001 Brian Paul All Rights Reserved.");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Permission is hereby granted, free of charge, to any person obtaining a");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "copy of this software and associated documentation files (the \"Software\"),");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "to deal in the Software without restriction, including without limitation");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "the rights to use, copy, modify, merge, publish, distribute, sublicense,");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "and/or sell copies of the Software, and to permit persons to whom the");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "Software is furnished to do so, subject to the following conditions:");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "The above copyright notice and this permission notice shall be included");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "in all copies or substantial portions of the Software.");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN");
	vkk_uiTextBox_printf(self->textbox_license, "%s", "THE SOFTWARE.");

	vkk_uiListBox_t* page   = vkk_uiWindow_page(window);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->gb_renderer);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->text_intro);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->textbox_intro);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->linkbox_github);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->text_license);
	vkk_uiListBox_add(page, (vkk_uiWidget_t*) self->textbox_license);

	// success
	return self;

	// failure
	fail_linkbox_github:
		vkk_uiTextBox_delete(&self->textbox_license);
	fail_textbox_license:
		vkk_uiTextBox_delete(&self->textbox_intro);
	fail_textbox_intro:
		vkk_uiText_delete(&self->text_license);
	fail_text_license:
		vkk_uiText_delete(&self->text_intro);
	fail_text_intro:
		vkk_uiGraphicsBox_delete(&self->gb_renderer);
	fail_gb_renderer:
		vkk_uiWindow_delete((vkk_uiWindow_t**) &self);
	return NULL;
}

void gears_viewAbout_delete(gears_viewAbout_t** _self)
{
	ASSERT(_self);

	gears_viewAbout_t* self = *_self;
	if(self)
	{
		vkk_uiWindow_t*  window = (vkk_uiWindow_t*) self;
		vkk_uiListBox_t* page   = vkk_uiWindow_page(window);
		vkk_uiListBox_clear(page);
		vkk_uiTextBox_delete(&self->linkbox_github);
		vkk_uiTextBox_delete(&self->textbox_license);
		vkk_uiTextBox_delete(&self->textbox_intro);
		vkk_uiText_delete(&self->text_license);
		vkk_uiText_delete(&self->text_intro);
		vkk_uiGraphicsBox_delete(&self->gb_renderer);
		vkk_uiWindow_delete((vkk_uiWindow_t**) _self);
	}
}
