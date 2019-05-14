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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "gears_renderer.h"

#define LOG_TAG "gears"
#include "a3d/a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

/***********************************************************
* main                                                     *
***********************************************************/

int main(int argc, char** argv)
{
	uint32_t version = VK_MAKE_VERSION(1,0,0);
	gears_renderer_t* renderer;
	renderer = gears_renderer_new(NULL,
	                              "GearsVK",
	                              version);
	if(renderer == NULL)
	{
		return EXIT_FAILURE;
	}

	int running = 1;
	while(running)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_KEYUP)
			{
				if(e.key.keysym.sym == SDLK_ESCAPE)
				{
					running = 0;
				}
			}
			else if(e.type == SDL_QUIT)
			{
				running = 0;
			}
		}

		gears_renderer_draw(renderer);
	}

	gears_renderer_delete(&renderer);

	// success
	return EXIT_SUCCESS;
}
