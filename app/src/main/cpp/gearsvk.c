/*
 * Copyright (c) 2019 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "gears"
#include "libcc/cc_log.h"
#include "libcc/cc_timestamp.h"
#include "libvkk/vkk.h"
#include "libvkk/vkui/vkui_key.h"
#include "gears_renderer.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int gRunning = 1;

static void cmd_fn(int cmd, const char* msg)
{
	if(cmd == GEARS_CMD_EXIT)
	{
		gRunning = 0;
	}
}

static int keyPress(SDL_Keysym* keysym,
                    int* keycode, int* meta)
{
	assert(keysym);
	assert(keycode);
	assert(meta);

	// convert the keycode
	// TODO - INSERT
	*keycode = 0;
	*meta    = 0;
	if((keysym->sym >= SDLK_a) &&
	   (keysym->sym <= SDLK_z))
	{
		int a = (int) 'a';
		*keycode = a + (keysym->sym - SDLK_a);
	}
	else if((keysym->sym >= SDLK_0) &&
	        (keysym->sym <= SDLK_9))
	{
		int z = (int) '0';
		*keycode = z + (keysym->sym - SDLK_0);
	}
	else if((keysym->sym >= SDLK_KP_0) &&
	        (keysym->sym <= SDLK_KP_9))
	{
		int z = (int) '0';
		*keycode = z + (keysym->sym - SDLK_KP_0);
	}
	else if((keysym->sym == SDLK_RETURN) ||
	        (keysym->sym == SDLK_KP_ENTER))
	{
		*keycode = VKUI_KEY_ENTER;
	}
	else if(keysym->sym == SDLK_ESCAPE)
	{
		*keycode = VKUI_KEY_ESCAPE;
	}
	else if(keysym->sym == SDLK_BACKSPACE)
	{
		*keycode = VKUI_KEY_BACKSPACE;
	}
	else if(keysym->sym == SDLK_DELETE)
	{
		*keycode = VKUI_KEY_DELETE;
	}
	else if(keysym->sym == SDLK_UP)
	{
		*keycode = VKUI_KEY_UP;
	}
	else if(keysym->sym == SDLK_DOWN)
	{
		*keycode = VKUI_KEY_DOWN;
	}
	else if(keysym->sym == SDLK_LEFT)
	{
		*keycode = VKUI_KEY_LEFT;
	}
	else if(keysym->sym == SDLK_RIGHT)
	{
		*keycode = VKUI_KEY_RIGHT;
	}
	else if(keysym->sym == SDLK_HOME)
	{
		*keycode = VKUI_KEY_HOME;
	}
	else if(keysym->sym == SDLK_END)
	{
		*keycode = VKUI_KEY_END;
	}
	else if(keysym->sym == SDLK_PAGEUP)
	{
		*keycode = VKUI_KEY_PGUP;
	}
	else if(keysym->sym == SDLK_PAGEDOWN)
	{
		*keycode = VKUI_KEY_PGDOWN;
	}
	else if(keysym->sym == SDLK_ESCAPE)
	{
		*keycode = VKUI_KEY_ESCAPE;
	}
	else if(keysym->sym == SDLK_BACKSPACE)
	{
		*keycode = (int) '\b';
	}
	else if(keysym->sym == SDLK_TAB)
	{
		*keycode = (int) '\t';
	}
	else if(keysym->sym == SDLK_SPACE)
	{
		*keycode = (int) ' ';
	}
	else if(keysym->sym == SDLK_EXCLAIM)
	{
		*keycode = (int) '!';
	}
	else if(keysym->sym == SDLK_QUOTEDBL)
	{
		*keycode = (int) '"';
	}
	else if(keysym->sym == SDLK_HASH)
	{
		*keycode = (int) '#';
	}
	else if(keysym->sym == SDLK_DOLLAR)
	{
		*keycode = (int) '$';
	}
	else if(keysym->sym == SDLK_AMPERSAND)
	{
		*keycode = (int) '&';
	}
	else if(keysym->sym == SDLK_QUOTE)
	{
		*keycode = (int) '\'';
	}
	else if(keysym->sym == SDLK_LEFTPAREN)
	{
		*keycode = (int) '(';
	}
	else if(keysym->sym == SDLK_RIGHTPAREN)
	{
		*keycode = (int) ')';
	}
	else if((keysym->sym == SDLK_ASTERISK) ||
	        (keysym->sym == SDLK_KP_MULTIPLY))
	{
		*keycode = (int) '*';
	}
	else if((keysym->sym == SDLK_PLUS) ||
	        (keysym->sym == SDLK_KP_PLUS))
	{
		*keycode = (int) '+';
	}
	else if(keysym->sym == SDLK_COMMA)
	{
		*keycode = (int) ',';
	}
	else if((keysym->sym == SDLK_MINUS) ||
	        (keysym->sym == SDLK_KP_MINUS))
	{
		*keycode = (int) '-';
	}
	else if((keysym->sym == SDLK_PERIOD) ||
	        (keysym->sym == SDLK_KP_PERIOD))
	{
		*keycode = (int) '.';
	}
	else if((keysym->sym == SDLK_SLASH) ||
	        (keysym->sym == SDLK_KP_DIVIDE))
	{
		*keycode = (int) '/';
	}
	else if(keysym->sym == SDLK_COLON)
	{
		*keycode = (int) ':';
	}
	else if(keysym->sym == SDLK_SEMICOLON)
	{
		*keycode = (int) ';';
	}
	else if(keysym->sym == SDLK_LESS)
	{
		*keycode = (int) '<';
	}
	else if((keysym->sym == SDLK_EQUALS) ||
	        (keysym->sym == SDLK_KP_EQUALS))
	{
		*keycode = (int) '=';
	}
	else if(keysym->sym == SDLK_GREATER)
	{
		*keycode = (int) '>';
	}
	else if(keysym->sym == SDLK_QUESTION)
	{
		*keycode = (int) '?';
	}
	else if(keysym->sym == SDLK_AT)
	{
		*keycode = (int) '@';
	}
	else if(keysym->sym == SDLK_LEFTBRACKET)
	{
		*keycode = (int) '[';
	}
	else if(keysym->sym == SDLK_BACKSLASH)
	{
		*keycode = (int) '\\';
	}
	else if(keysym->sym == SDLK_RIGHTBRACKET)
	{
		*keycode = (int) ']';
	}
	else if(keysym->sym == SDLK_CARET)
	{
		*keycode = (int) '^';
	}
	else if(keysym->sym == SDLK_UNDERSCORE)
	{
		*keycode = (int) '_';
	}
	else if(keysym->sym == SDLK_BACKQUOTE)
	{
		*keycode = (int) '`';
	}
	else if(keysym->sym == SDLK_BACKQUOTE)
	{
		*keycode = (int) '`';
	}
	else
	{
		// ignore
		LOGD("sym=0x%X", keysym->sym);
		return 0;
	}

	// convert the meta
	if(keysym->mod & KMOD_ALT)
	{
		*meta |= VKUI_KEY_ALT;
	}
	if(keysym->mod & KMOD_LALT)
	{
		*meta |= VKUI_KEY_ALT_L;
	}
	if(keysym->mod & KMOD_RALT)
	{
		*meta |= VKUI_KEY_ALT_R;
	}
	if(keysym->mod & KMOD_CTRL)
	{
		*meta |= VKUI_KEY_CTRL;
	}
	if(keysym->mod & KMOD_LCTRL)
	{
		*meta |= VKUI_KEY_CTRL_L;
	}
	if(keysym->mod & KMOD_RCTRL)
	{
		*meta |= VKUI_KEY_CTRL_R;
	}
	if(keysym->mod & KMOD_SHIFT)
	{
		*meta |= VKUI_KEY_SHIFT;
	}
	if(keysym->mod & KMOD_LSHIFT)
	{
		*meta |= VKUI_KEY_SHIFT_L;
	}
	if(keysym->mod & KMOD_RSHIFT)
	{
		*meta |= VKUI_KEY_SHIFT_R;
	}
	if(keysym->mod & KMOD_CAPS)
	{
		*meta |= VKUI_KEY_CAPS;
	}

	return 1;
}

/***********************************************************
* main                                                     *
***********************************************************/

int main(int argc, char** argv)
{
	uint32_t version = VKK_MAKE_VERSION(1,0,1);
	gears_renderer_t* renderer;
	renderer = gears_renderer_new(NULL,
	                              "GearsVK",
	                              version,
	                              cmd_fn);
	if(renderer == NULL)
	{
		return EXIT_FAILURE;
	}

	while(gRunning)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			int keycode = 0;
			int meta    = 0;
			if((e.type == SDL_KEYUP) ||
			   ((e.type == SDL_KEYDOWN) && (e.key.repeat)))
			{
				if(keyPress(&e.key.keysym, &keycode, &meta))
				{
					gears_renderer_keyPress(renderer,
					                        keycode, meta);
				}
			}
			else if(e.type == SDL_MOUSEBUTTONUP)
			{
				float  x  = (float) e.button.x;
				float  y  = (float) e.button.y;
				double ts = cc_timestamp();
				gears_renderer_touch(renderer,
				                     GEARS_TOUCH_ACTION_UP,
				                     1, ts, x, y,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f);
			}
			else if(e.type == SDL_MOUSEBUTTONDOWN)
			{
				float  x  = (float) e.button.x;
				float  y  = (float) e.button.y;
				double ts = cc_timestamp();
				gears_renderer_touch(renderer,
				                     GEARS_TOUCH_ACTION_DOWN,
				                     1, ts, x, y,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f);
			}
			else if(e.type == SDL_MOUSEMOTION)
			{
				float  x  = (float) e.button.x;
				float  y  = (float) e.button.y;
				double ts = cc_timestamp();
				gears_renderer_touch(renderer,
				                     GEARS_TOUCH_ACTION_MOVE,
				                     1, ts, x, y,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f,
				                     0.0f, 0.0f);
			}
			else if(e.type == SDL_WINDOWEVENT)
			{
				if(e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					if(gears_renderer_resize(renderer) == 0)
					{
						gears_renderer_delete(&renderer);
						return EXIT_FAILURE;
					}
				}
			}
			else if(e.type == SDL_QUIT)
			{
				gRunning = 0;
			}
		}

		gears_renderer_draw(renderer);
	}

	gears_renderer_delete(&renderer);

	// success
	return EXIT_SUCCESS;
}
