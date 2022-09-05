/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_layerHud_H
#define gears_layerHud_H

#include "libvkk/vkk_ui.h"

struct gears_overlay_s;

typedef struct
{
	vkk_uiWindow_t     base;
	vkk_uiBulletBox_t* bulletbox_about;
	vkk_uiText_t*      text_fps;

	// cached state
	int fps;
} gears_layerHud_t;

gears_layerHud_t* gears_layerHud_new(struct gears_overlay_s* overlay);
void              gears_layerHud_delete(gears_layerHud_t** _self);
void              gears_layerHud_updateFps(gears_layerHud_t* self,
                                           int fps);

#endif
