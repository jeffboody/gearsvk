/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_viewAbout_H
#define gears_viewAbout_H

#include "libvkk/vkk_ui.h"

typedef struct gears_overlay_s gears_overlay_t;

typedef struct
{
	vkk_uiWindow_t       base;
	vkk_uiGraphicsBox_t* gb_renderer;
	vkk_uiText_t*        text_intro;
	vkk_uiText_t*        text_license;
	vkk_uiTextBox_t*     textbox_intro;
	vkk_uiTextBox_t*     textbox_license;
	vkk_uiTextBox_t*     linkbox_github;
} gears_viewAbout_t;

gears_viewAbout_t* gears_viewAbout_new(gears_overlay_t* overlay);
void               gears_viewAbout_delete(gears_viewAbout_t** _self);

#endif
