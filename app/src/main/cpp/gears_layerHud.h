/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_layerHud_H
#define gears_layerHud_H

#include "libvkk/vkk_ui.h"

typedef struct gears_overlay_s gears_overlay_t;

typedef struct
{
	vkk_uiWindow_t       base;
	vkk_uiBulletBox_t*   bulletbox_about;
	vkk_uiText_t*        text_fps;
	vkk_uiGraphicsBox_t* gb_renderer;

	int last_fps;
} gears_layerHud_t;

gears_layerHud_t* gears_layerHud_new(gears_overlay_t* overlay);
void              gears_layerHud_delete(gears_layerHud_t** _self);

#endif
