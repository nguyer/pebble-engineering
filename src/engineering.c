#include <pebble.h>
#include "engineering.h"

#define KEY_TEMPERATURE 0

static Window *window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;

static GPath *s_minute_arrow, *s_hour_arrow;
static char s_date_buffer[7], s_temp_buffer[3];
static GDrawCommandImage *s_command_image_hour_marks;
static GDrawCommandImage *s_command_image_minute_marks;

static AppSync s_sync;
static uint8_t s_sync_buffer[64];

GColor BACKGROUND_COLOR;
GColor HOUR_MARK_COLOR;
GColor MINUTE_MARK_COLOR;
GColor NUMBER_COLOR;
GColor HOUR_HAND_COLOR;
GColor MINUTE_HAND_COLOR;
GColor SECOND_HAND_COLOR;

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	switch (key) {
		case KEY_TEMPERATURE:
		if (strlen(new_tuple->value->cstring) >= 1) {
			strcpy(s_temp_buffer, new_tuple->value->cstring);
			strcat(s_temp_buffer, "°");
		}
		break;
	}
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static bool color_hour_marks(GDrawCommand *command, uint32_t index, void *context) {
  gdraw_command_set_stroke_color(command, HOUR_MARK_COLOR);
  return true;
}

static bool color_minute_marks(GDrawCommand *command, uint32_t index, void *context) {
  gdraw_command_set_stroke_color(command, MINUTE_MARK_COLOR);
  return true;
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
	GPoint origin = GPoint(0,0);
	if (s_command_image_hour_marks) {
		// Draw it
		gdraw_command_list_iterate(gdraw_command_image_get_command_list(s_command_image_hour_marks), color_hour_marks, &ctx);
		gdraw_command_image_draw(ctx, s_command_image_hour_marks, origin);
	}
	if (s_command_image_minute_marks) {
		// Draw it
		gdraw_command_list_iterate(gdraw_command_image_get_command_list(s_command_image_minute_marks), color_minute_marks, &ctx);
		gdraw_command_image_draw(ctx, s_command_image_minute_marks, origin);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	int16_t second_hand_length = bounds.size.w / 2 - 5;
	int16_t second_hand_tail_length = 15;

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	int32_t second_angle_tail = TRIG_MAX_ANGLE * (t->tm_sec + 30) / 60;
	
	GPoint second_hand = {
		.x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
	};
	GPoint second_hand_tail = {
		.x = (int16_t)(sin_lookup(second_angle_tail) * (int32_t)second_hand_tail_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle_tail) * (int32_t)second_hand_tail_length / TRIG_MAX_RATIO) + center.y,
	};

	// numbers
	graphics_context_set_text_color(ctx, NUMBER_COLOR);
	graphics_draw_text(ctx, "12", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(63, 18, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, "1", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(85, 23, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	graphics_draw_text(ctx, "2", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(104, 43, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	graphics_draw_text(ctx, "3", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(112, 68, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	graphics_draw_text(ctx, "4", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(104, 93, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	graphics_draw_text(ctx, "5", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(85, 110, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	graphics_draw_text(ctx, "6", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(62, 118, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, "7", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(39, 110, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, "8", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(20, 93, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, "9", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(14, 68, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, "10", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(20, 43, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, "11", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(39, 23, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

	// date
	graphics_draw_text(ctx, s_date_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(80, 75, 40, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	// temperature
	graphics_draw_text(ctx, s_temp_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(27, 75, 40, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	// minute hand
	graphics_context_set_fill_color(ctx, MINUTE_HAND_COLOR);

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	gpath_draw_filled(ctx, s_minute_arrow);

	// hour hand
	graphics_context_set_fill_color(ctx, HOUR_HAND_COLOR);
	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	gpath_draw_filled(ctx, s_hour_arrow);

	// second hand
	graphics_context_set_stroke_color(ctx, SECOND_HAND_COLOR);
	graphics_draw_line(ctx, second_hand, center);
	graphics_draw_line(ctx, second_hand_tail, center);

	// dot in the middle
	//graphics_context_set_stroke_color(ctx, GColorDarkCandyAppleRed);
	graphics_context_set_fill_color(ctx, SECOND_HAND_COLOR);
	graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);
	//graphics_draw_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	
	strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d", t);
	uppercase(s_date_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_simple_bg_layer = layer_create(bounds);
	layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
	layer_add_child(window_layer, s_simple_bg_layer);

	window_set_background_color(window, BACKGROUND_COLOR);

	// Load the image and check it was succcessful
	s_command_image_hour_marks = gdraw_command_image_create_with_resource(RESOURCE_ID_HOUR_MARKS);
	if (!s_command_image_hour_marks) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Image is NULL!");
	}
	s_command_image_minute_marks = gdraw_command_image_create_with_resource(RESOURCE_ID_MINUTE_MARKS);
	if (!s_command_image_minute_marks) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Image is NULL!");
	}

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
	
	Tuplet initial_values[] = {
		TupletCString(KEY_TEMPERATURE, "\0")
	};

	app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
				  initial_values, ARRAY_LENGTH(initial_values),
				  sync_tuple_changed_callback, sync_error_callback, NULL);

}

static void window_unload(Window *window) {
	layer_destroy(s_simple_bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_day_label);
	text_layer_destroy(s_num_label);

	layer_destroy(s_hands_layer);

	// Destroy the image
	gdraw_command_image_destroy(s_command_image_hour_marks);
	gdraw_command_image_destroy(s_command_image_minute_marks);
}

static void init() {
	
	BACKGROUND_COLOR = GColorBlack;
	HOUR_MARK_COLOR = GColorLightGray; //GColorLightGray;
	MINUTE_MARK_COLOR = GColorDarkGray;
	NUMBER_COLOR = GColorLightGray;
	HOUR_HAND_COLOR = GColorRed;
	MINUTE_HAND_COLOR = GColorWhite;
	SECOND_HAND_COLOR = GColorRed;
	
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);

	s_temp_buffer[0] = '\0';
	s_date_buffer[0] = '\0';
	
	// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);

	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	
	app_message_open(64, 64);
}

static void deinit() {
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);
	
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main() {
	init();
	app_event_loop();
	deinit();
}

char *uppercase(char *str) {
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 0x20;
        }
    }

    return str;
}