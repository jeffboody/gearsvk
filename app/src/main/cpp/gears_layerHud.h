/*
 * Copyright (c) 2018 Jeff Boody
 */

#ifndef gears_layerHud_H
#define gears_layerHud_H

#include "libcc/math/cc_vec4f.h"
#include "libvkk/vkui/vkui_layer.h"
#include "libvkk/vkui/vkui.h"

struct gears_overlay_s;

typedef struct
{
	vkui_layer_t layer;

	// widgets
	vkui_bulletbox_t* bulletbox_about;
	vkui_text_t*      text_fps;

	// cached state
	int fps;
} gears_layerHud_t;

gears_layerHud_t* gears_layerHud_new(struct gears_overlay_s* overlay);
void              gears_layerHud_delete(gears_layerHud_t** _self);
void              gears_layerHud_updateFps(gears_layerHud_t* self,
                                           int fps);

#endif
