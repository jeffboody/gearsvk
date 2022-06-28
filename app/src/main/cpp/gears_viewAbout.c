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

static int clickBack(vkk_uiWidget_t* widget,
                     int state,
                     float x, float y)
{
	ASSERT(widget);

	if(state == VKK_UI_WIDGET_POINTER_UP)
	{
		vkk_uiScreen_t* screen = widget->screen;
		return vkk_uiScreen_windowPop(screen);
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

	vkk_uiScreen_t* screen = overlay->screen;

	vkk_uiWindowInfo_t info =
	{
		.flags = VKK_UI_WINDOW_FLAG_TITLE |
		         VKK_UI_WINDOW_FLAG_PAGE_DEFAULT,
		.label = "About",
		.fn =
		{
			.click_fn = clickBack
		},
	};

	gears_viewAbout_t* self;
	self = (gears_viewAbout_t*)
	       vkk_uiWindow_new(screen,
	                        sizeof(gears_viewAbout_t),
	                        &info);
	if(self == NULL)
	{
		return NULL;
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

	self->textbox_intro = vkk_uiTextbox_newPageParagraph(screen);
	if(self->textbox_intro == NULL)
	{
		goto fail_textbox_intro;
	}

	self->textbox_license = vkk_uiTextbox_newPageParagraph(screen);
	if(self->textbox_license == NULL)
	{
		goto fail_textbox_license;
	}

	vkk_uiWidgetFn_t linkbox_github_fn =
	{
		.msg      = "https://github.com/jeffboody/gearsvk/",
		.click_fn = vkk_uiWidget_clickUrlFn
	};

	self->linkbox_github = vkk_uiTextbox_newPageButton(screen,
	                                                   &linkbox_github_fn);
	if(self->linkbox_github == NULL)
	{
		goto fail_linkbox_github;
	}

	vkk_uiText_label(self->text_intro, "%s", "About");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "Gears VK is a heavily modified port of the famous \"gears\" demo.");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "The Gears demo is an open source project intended to help developers to");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "learn graphics APIs. The Gears demo was originally");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "written by Brian Paul as part of the Mesa3D project.");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "I've ported this app between multiple languages and graphics APIs over the");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "years and have found it a very useful learning tool.");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "");
	vkk_uiTextbox_printf(self->textbox_intro, "%s", "Source code is available on Github.");
	vkk_uiTextbox_printf(self->linkbox_github, "%s", "https://github.com/jeffboody/gearsvk/");
	vkk_uiTextbox_printf(self->linkbox_github, "%s", "");

	vkk_uiText_label(self->text_license, "%s", "License");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "This port of Gears VK was implemented by");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Jeff Boody (jeffboody@gmail.com) under The MIT License.");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Copyright (c) 2009-2011 Jeff Boody");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Gears for Android is a heavily modified port of the famous \"gears\" demo. ");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "As such, it is a derived work subject to the license requirements (below) ");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "of the original work.");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Copyright (c) 1999-2001 Brian Paul All Rights Reserved.");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Permission is hereby granted, free of charge, to any person obtaining a");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "copy of this software and associated documentation files (the \"Software\"),");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "to deal in the Software without restriction, including without limitation");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "the rights to use, copy, modify, merge, publish, distribute, sublicense,");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "and/or sell copies of the Software, and to permit persons to whom the");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "Software is furnished to do so, subject to the following conditions:");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "The above copyright notice and this permission notice shall be included");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "in all copies or substantial portions of the Software.");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN");
	vkk_uiTextbox_printf(self->textbox_license, "%s", "THE SOFTWARE.");

	vkk_uiWindow_t*  window = (vkk_uiWindow_t*) self;
	vkk_uiListbox_t* page   = vkk_uiWindow_page(window);
	vkk_uiListbox_add(page, (vkk_uiWidget_t*) self->text_intro);
	vkk_uiListbox_add(page, (vkk_uiWidget_t*) self->textbox_intro);
	vkk_uiListbox_add(page, (vkk_uiWidget_t*) self->linkbox_github);
	vkk_uiListbox_add(page, (vkk_uiWidget_t*) self->text_license);
	vkk_uiListbox_add(page, (vkk_uiWidget_t*) self->textbox_license);

	// success
	return self;

	// failure
	fail_linkbox_github:
		vkk_uiTextbox_delete(&self->textbox_license);
	fail_textbox_license:
		vkk_uiTextbox_delete(&self->textbox_intro);
	fail_textbox_intro:
		vkk_uiText_delete(&self->text_license);
	fail_text_license:
		vkk_uiText_delete(&self->text_intro);
	fail_text_intro:
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
		vkk_uiListbox_t* page   = vkk_uiWindow_page(window);
		vkk_uiListbox_clear(page);
		vkk_uiTextbox_delete(&self->linkbox_github);
		vkk_uiTextbox_delete(&self->textbox_license);
		vkk_uiTextbox_delete(&self->textbox_intro);
		vkk_uiText_delete(&self->text_license);
		vkk_uiText_delete(&self->text_intro);
		vkk_uiWindow_delete((vkk_uiWindow_t**) _self);
	}
}
