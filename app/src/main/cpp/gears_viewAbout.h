/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_viewAbout_H
#define gears_viewAbout_H

#include "libvkk/vkui/vkui_viewbox.h"
#include "libvkk/vkui/vkui.h"

struct gears_overlay_s;

typedef struct
{
	vkui_viewbox_t  viewbox;
	vkui_listbox_t* listbox;

	// headings
	vkui_text_t* text_intro;
	vkui_text_t* text_icons;
	vkui_text_t* text_barlow;
	vkui_text_t* text_expat;
	vkui_text_t* text_license;

	// paragraphs
	vkui_textbox_t* textbox_intro;
	vkui_textbox_t* textbox_icons;
	vkui_textbox_t* textbox_barlow;
	vkui_textbox_t* textbox_expat;
	vkui_textbox_t* textbox_license;

	// links
	vkui_textbox_t* linkbox_github;
	vkui_textbox_t* linkbox_icons;
	vkui_textbox_t* linkbox_barlow;
	vkui_textbox_t* linkbox_expat;
} gears_viewAbout_t;

gears_viewAbout_t* gears_viewAbout_new(struct gears_overlay_s* overlay,
                                       vkui_widget_clickFn clickBack);
void               gears_viewAbout_delete(gears_viewAbout_t** _self);

#endif
