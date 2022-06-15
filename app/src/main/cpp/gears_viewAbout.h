/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_viewAbout_H
#define gears_viewAbout_H

#include "libvkk/vkui.h"

struct gears_overlay_s;

typedef struct
{
	vkui_window_t   base;
	vkui_text_t*    text_intro;
	vkui_text_t*    text_license;
	vkui_textbox_t* textbox_intro;
	vkui_textbox_t* textbox_license;
	vkui_textbox_t* linkbox_github;
} gears_viewAbout_t;

gears_viewAbout_t* gears_viewAbout_new(struct gears_overlay_s* overlay);
void               gears_viewAbout_delete(gears_viewAbout_t** _self);

#endif
