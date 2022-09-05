/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_viewAbout_H
#define gears_viewAbout_H

#include "libvkk/vkk_ui.h"

struct gears_overlay_s;

typedef struct
{
	vkk_uiWindow_t   base;
	vkk_uiText_t*    text_intro;
	vkk_uiText_t*    text_license;
	vkk_uiTextBox_t* textbox_intro;
	vkk_uiTextBox_t* textbox_license;
	vkk_uiTextBox_t* linkbox_github;
} gears_viewAbout_t;

gears_viewAbout_t* gears_viewAbout_new(struct gears_overlay_s* overlay);
void               gears_viewAbout_delete(gears_viewAbout_t** _self);

#endif
