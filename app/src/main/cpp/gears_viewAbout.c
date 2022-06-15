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

/***********************************************************
* private                                                  *
***********************************************************/

static int clickBack(vkui_widget_t* widget,
                     int state,
                     float x, float y)
{
	ASSERT(widget);

	if(state == VKUI_WIDGET_POINTER_UP)
	{
		vkui_screen_t* screen = widget->screen;
		return vkui_screen_windowPop(screen);
	}
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

gears_viewAbout_t*
gears_viewAbout_new(struct gears_overlay_s* overlay)
{
	ASSERT(overlay);

	vkui_screen_t* screen = overlay->screen;

	vkui_windowInfo_t info =
	{
		.flags = VKUI_WINDOW_FLAG_TITLE |
		         VKUI_WINDOW_FLAG_PAGE_DEFAULT,
		.label = "About",
		.fn =
		{
			.click_fn = clickBack
		},
	};

	gears_viewAbout_t* self;
	self = (gears_viewAbout_t*)
	       vkui_window_new(screen,
	                       sizeof(gears_viewAbout_t),
	                       &info);
	if(self == NULL)
	{
		return NULL;
	}

	self->text_intro = vkui_text_newPageHeading(screen);
	if(self->text_intro == NULL)
	{
		goto fail_text_intro;
	}

	self->text_license = vkui_text_newPageHeading(screen);
	if(self->text_license == NULL)
	{
		goto fail_text_license;
	}

	self->textbox_intro = vkui_textbox_newPageParagraph(screen);
	if(self->textbox_intro == NULL)
	{
		goto fail_textbox_intro;
	}

	self->textbox_license = vkui_textbox_newPageParagraph(screen);
	if(self->textbox_license == NULL)
	{
		goto fail_textbox_license;
	}

	vkui_widgetFn_t linkbox_github_fn =
	{
		.msg      = "https://github.com/jeffboody/gearsvk/",
		.click_fn = vkui_widget_clickUrlFn
	};

	self->linkbox_github = vkui_textbox_newPageButton(screen,
	                                                  &linkbox_github_fn);
	if(self->linkbox_github == NULL)
	{
		goto fail_linkbox_github;
	}

	vkui_text_label(self->text_intro, "%s", "About");
	vkui_textbox_printf(self->textbox_intro, "%s", "Gears VK is a heavily modified port of the famous \"gears\" demo.");
	vkui_textbox_printf(self->textbox_intro, "%s", "");
	vkui_textbox_printf(self->textbox_intro, "%s", "The Gears demo is an open source project intended to help developers to");
	vkui_textbox_printf(self->textbox_intro, "%s", "learn graphics APIs. The Gears demo was originally");
	vkui_textbox_printf(self->textbox_intro, "%s", "written by Brian Paul as part of the Mesa3D project.");
	vkui_textbox_printf(self->textbox_intro, "%s", "I've ported this app between multiple languages and graphics APIs over the");
	vkui_textbox_printf(self->textbox_intro, "%s", "years and have found it a very useful learning tool.");
	vkui_textbox_printf(self->textbox_intro, "%s", "");
	vkui_textbox_printf(self->textbox_intro, "%s", "Source code is available on Github.");
	vkui_textbox_printf(self->linkbox_github, "%s", "https://github.com/jeffboody/gearsvk/");
	vkui_textbox_printf(self->linkbox_github, "%s", "");

	vkui_text_label(self->text_license, "%s", "License");
	vkui_textbox_printf(self->textbox_license, "%s", "This port of Gears VK was implemented by");
	vkui_textbox_printf(self->textbox_license, "%s", "Jeff Boody (jeffboody@gmail.com) under The MIT License.");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "Copyright (c) 2009-2011 Jeff Boody");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "Gears for Android is a heavily modified port of the famous \"gears\" demo. ");
	vkui_textbox_printf(self->textbox_license, "%s", "As such, it is a derived work subject to the license requirements (below) ");
	vkui_textbox_printf(self->textbox_license, "%s", "of the original work.");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "Copyright (c) 1999-2001 Brian Paul All Rights Reserved.");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "Permission is hereby granted, free of charge, to any person obtaining a");
	vkui_textbox_printf(self->textbox_license, "%s", "copy of this software and associated documentation files (the \"Software\"),");
	vkui_textbox_printf(self->textbox_license, "%s", "to deal in the Software without restriction, including without limitation");
	vkui_textbox_printf(self->textbox_license, "%s", "the rights to use, copy, modify, merge, publish, distribute, sublicense,");
	vkui_textbox_printf(self->textbox_license, "%s", "and/or sell copies of the Software, and to permit persons to whom the");
	vkui_textbox_printf(self->textbox_license, "%s", "Software is furnished to do so, subject to the following conditions:");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "The above copyright notice and this permission notice shall be included");
	vkui_textbox_printf(self->textbox_license, "%s", "in all copies or substantial portions of the Software.");
	vkui_textbox_printf(self->textbox_license, "%s", "");
	vkui_textbox_printf(self->textbox_license, "%s", "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
	vkui_textbox_printf(self->textbox_license, "%s", "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
	vkui_textbox_printf(self->textbox_license, "%s", "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE");
	vkui_textbox_printf(self->textbox_license, "%s", "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
	vkui_textbox_printf(self->textbox_license, "%s", "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
	vkui_textbox_printf(self->textbox_license, "%s", "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN");
	vkui_textbox_printf(self->textbox_license, "%s", "THE SOFTWARE.");

	vkui_window_t*  window = (vkui_window_t*) self;
	vkui_listbox_t* page   = vkui_window_page(window);
	vkui_listbox_add(page, (vkui_widget_t*) self->text_intro);
	vkui_listbox_add(page, (vkui_widget_t*) self->textbox_intro);
	vkui_listbox_add(page, (vkui_widget_t*) self->linkbox_github);
	vkui_listbox_add(page, (vkui_widget_t*) self->text_license);
	vkui_listbox_add(page, (vkui_widget_t*) self->textbox_license);

	// success
	return self;

	// failure
	fail_linkbox_github:
		vkui_textbox_delete(&self->textbox_license);
	fail_textbox_license:
		vkui_textbox_delete(&self->textbox_intro);
	fail_textbox_intro:
		vkui_text_delete(&self->text_license);
	fail_text_license:
		vkui_text_delete(&self->text_intro);
	fail_text_intro:
		vkui_window_delete((vkui_window_t**) &self);
	return NULL;
}

void gears_viewAbout_delete(gears_viewAbout_t** _self)
{
	ASSERT(_self);

	gears_viewAbout_t* self = *_self;
	if(self)
	{
		vkui_window_t*  window = (vkui_window_t*) self;
		vkui_listbox_t* page   = vkui_window_page(window);
		vkui_listbox_clear(page);
		vkui_textbox_delete(&self->linkbox_github);
		vkui_textbox_delete(&self->textbox_license);
		vkui_textbox_delete(&self->textbox_intro);
		vkui_text_delete(&self->text_license);
		vkui_text_delete(&self->text_intro);
		vkui_window_delete((vkui_window_t**) _self);
	}
}
