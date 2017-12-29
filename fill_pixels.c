/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "android_native_app_glue.h"
#include <android/log.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // rand

#define  LOG_TAG    "fill_pixels"
#define  LOG(level, format, ...) \
	__android_log_print(level, LOG_TAG, "[%s:%d]\n" format, __func__, __LINE__, ##__VA_ARGS__)

typedef uint16_t color_16bits_t;
typedef uint8_t  color_8bits_channel_t;

#define make565(r,g,b) \
	((color_16bits_t) \
	 ((r >> 3) << 11) |\
	 ((g >> 2) << 5)  |\
	  (b >> 3) \
	)


typedef uint16_t window_pixel_t;

static inline window_pixel_t * buffer_first_pixel_of_next_line
(ANativeWindow_Buffer const * __restrict const buffer,
 window_pixel_t       const * __restrict const line_start)
{
	return (window_pixel_t *) (line_start + buffer->stride);
}

#define PIXEL_COLORS_MAX 4
#define PIXEL_COLORS_MAX_MASK 0b11
static inline uint_fast32_t pixel_colors_next
(uint_fast32_t current_index)
{
	return (rand() & PIXEL_COLORS_MAX_MASK);
}

static void fill_pixels(ANativeWindow_Buffer* buffer)
{
	static color_16bits_t const pixel_colors[PIXEL_COLORS_MAX] = {
		make565(255,  0,  0),
		make565(  0,255,  0),
		make565(  0,  0,255),
		make565(255,255,  0)
	};

	/* Current pixel colors index */
	uint_fast32_t p_c = rand() & PIXEL_COLORS_MAX_MASK;
	color_16bits_t current_pixel_color;

	window_pixel_t * __restrict current_pixel = buffer->bits;

	uint_fast32_t const line_width  = buffer->width;
	uint_fast32_t const line_stride = buffer->stride;
	uint_fast32_t n_lines = buffer->height;
	while(n_lines--) {

		window_pixel_t const * __restrict const current_line_start =
			current_pixel;

		window_pixel_t const * __restrict const last_pixel_of_the_line =
			current_line_start + line_width;

		current_pixel_color = pixel_colors[p_c];

		while (current_pixel <= last_pixel_of_the_line) {
			*current_pixel = current_pixel_color;
			current_pixel++;
		}

		p_c = pixel_colors_next(p_c);
		current_pixel =	current_line_start + line_stride;
	}
}

struct engine {
	struct android_app* app;

	int animating;

	int32_t initial_window_format;
};

#ifndef __cplusplus
	enum bool { false, true };
	typedef enum bool bool;
#endif

static inline bool engine_have_a_window
(struct engine const * __restrict const engine)
{
	return engine->app->window != NULL;
}

static void engine_draw_frame(struct engine* engine) {

	ANativeWindow_Buffer buffer;

	if (!engine_have_a_window(engine))
	{
		LOG(ANDROID_LOG_WARN, "The engine doesn't have a window !?\n");
		goto draw_frame_end;
	}

	
	if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0)
	{
		LOG(ANDROID_LOG_WARN, "Could not lock the window... :C\n");
		goto draw_frame_end;
	}

	fill_pixels(&buffer);
	ANativeWindow_unlockAndPost(engine->app->window);

draw_frame_end:
	return;
}

static inline void engine_term_display
(struct engine * __restrict const engine)
{
	engine->animating = 0;
}

static int32_t engine_handle_input
(struct android_app * app, AInputEvent * event) {
	struct engine * const engine =
		(struct engine *) app->userData;

	int32_t const current_event_type =
		AInputEvent_getType(event);

	if (current_event_type == AINPUT_EVENT_TYPE_MOTION) {
		engine->animating = 1;
		return 1;
	} else if (current_event_type == AINPUT_EVENT_TYPE_KEY) {
		LOG(ANDROID_LOG_INFO,
			"Key event: action=%d keyCode=%d metaState=0x%x",
			AKeyEvent_getAction(event),
			AKeyEvent_getKeyCode(event),
			AKeyEvent_getMetaState(event));
	}

	return 0;
}

static void engine_handle_cmd
(struct android_app* app, int32_t cmd)
{
	struct engine * __restrict const engine =
		(struct engine *) app->userData;

	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			if (engine_have_a_window(engine))
			{

				engine->initial_window_format =
					ANativeWindow_getFormat(app->window);

				ANativeWindow_setBuffersGeometry(app->window,
					ANativeWindow_getWidth(app->window),
					ANativeWindow_getHeight(app->window),
					WINDOW_FORMAT_RGB_565);

				engine_draw_frame(engine);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			engine_term_display(engine);

			ANativeWindow_setBuffersGeometry(app->window,
				ANativeWindow_getWidth(app->window),
				ANativeWindow_getHeight(app->window),
				engine->initial_window_format);

			break;
		case APP_CMD_LOST_FOCUS:
			engine->animating = 0;
			engine_draw_frame(engine);
			break;
	}
}

void android_main(struct android_app* state) {

	struct engine engine = {0};

	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// loop waiting for stuff to do.

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while (
			(ident=ALooper_pollAll(
				engine.animating ? 0 : -1,
				NULL, &events, (void **) &source))
			>= 0)
		{

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				LOG(ANDROID_LOG_INFO,
					"Engine thread destroy requested!");
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			engine_draw_frame(&engine);
		}
	}
}

