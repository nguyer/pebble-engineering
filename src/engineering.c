#include <pebble.h>
#include <inttypes.h>
#include "engineering.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;

static GPath *s_minute_arrow, *s_hour_arrow;
static char s_date_buffer[7], s_temp_buffer[5];

static AppSync s_sync;
static uint8_t s_sync_buffer[64];

static GColor gcolor_background, gcolor_hour_marks, gcolor_minute_marks, gcolor_numbers, gcolor_hour_hand, gcolor_minute_hand, gcolor_second_hand;
static bool b_show_numbers, b_show_temperature, b_show_date, b_show_second_hand;

static void load_persisted_values() {
	// SHOW_NUMBERS
	if (persist_exists(KEY_SHOW_NUMBERS)) {
	  b_show_numbers = persist_read_int(KEY_SHOW_NUMBERS);
	}

	// SHOW_SECOND_HAND
	if (persist_exists(KEY_SHOW_SECOND_HAND)) {
	  b_show_second_hand = persist_read_int(KEY_SHOW_SECOND_HAND);
	}

	// SHOW_TEMPERATURE
	if (persist_exists(KEY_SHOW_TEMPERATURE)) {
	  b_show_temperature = persist_read_int(KEY_SHOW_TEMPERATURE);
	}

	// SHOW_DATE
	if (persist_exists(KEY_SHOW_DATE)) {
	  b_show_date = persist_read_int(KEY_SHOW_DATE);
	}

	// COLOR_BACKGROUND
	if (persist_exists(KEY_COLOR_BACKGROUND)) {
		int color_hex = persist_read_int(KEY_COLOR_BACKGROUND);
		gcolor_background = GColorFromHEX(color_hex);
		window_set_background_color(window, gcolor_background);
	}

	// COLOR_HOUR_MARKS
	if (persist_exists(KEY_COLOR_HOUR_MARKS)) {
		int color_hex = persist_read_int(KEY_COLOR_HOUR_MARKS);
		gcolor_hour_marks = GColorFromHEX(color_hex);
	}

	// COLOR_MINUTE_MARKS
	if (persist_exists(KEY_COLOR_MINUTE_MARKS)) {
		int color_hex = persist_read_int(KEY_COLOR_MINUTE_MARKS);
		gcolor_minute_marks = GColorFromHEX(color_hex);
	}

	// COLOR_LABEL
	if (persist_exists(KEY_COLOR_LABEL)) {
		int color_hex = persist_read_int(KEY_COLOR_LABEL);
		gcolor_numbers = GColorFromHEX(color_hex);
	}

	// COLOR_HOUR_HAND
	if (persist_exists(KEY_COLOR_HOUR_HAND)) {
		int color_hex = persist_read_int(KEY_COLOR_HOUR_HAND);
		gcolor_hour_hand = GColorFromHEX(color_hex);
	}

	// COLOR_MINUTE_HAND
	if (persist_exists(KEY_COLOR_MINUTE_HAND)) {
		int color_hex = persist_read_int(KEY_COLOR_MINUTE_HAND);
		gcolor_minute_hand = GColorFromHEX(color_hex);
	}

	// COLOR_SECOND_HAND
	if (persist_exists(KEY_COLOR_SECOND_HAND)) {
		int color_hex = persist_read_int(KEY_COLOR_SECOND_HAND);
		gcolor_second_hand = GColorFromHEX(color_hex);
	}

}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
 	Tuple *temperature_t = dict_find(iter, KEY_TEMPERATURE);
 	if(temperature_t) {
		snprintf(s_temp_buffer, 5, "%d°", temperature_t->value->int16);
		APP_LOG(APP_LOG_LEVEL_INFO, s_temp_buffer);
 	}

	Tuple *show_numbers_t = dict_find(iter, KEY_SHOW_NUMBERS);
	if(show_numbers_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Show numbers %d", show_numbers_t->value->uint8);
 		b_show_numbers = show_numbers_t->value->uint8;
		persist_write_int(KEY_SHOW_NUMBERS, b_show_numbers);
 	}

	Tuple *show_date_t = dict_find(iter, KEY_SHOW_DATE);
	if(show_date_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Show date %d", show_date_t->value->uint8);
 		b_show_date = show_date_t->value->uint8;
		persist_write_int(KEY_SHOW_DATE, show_date_t->value->uint8);
 	}

	Tuple *show_temperature_t = dict_find(iter, KEY_SHOW_TEMPERATURE);
	if(show_temperature_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Show temperature %d", show_temperature_t->value->uint8);
 		b_show_temperature = show_temperature_t->value->uint8;
		persist_write_int(KEY_SHOW_TEMPERATURE, b_show_temperature);
 	}

	Tuple *show_second_hand_t = dict_find(iter, KEY_SHOW_SECOND_HAND);
	if(show_second_hand_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Show second hand %d", show_second_hand_t->value->uint8);
 		b_show_second_hand = show_second_hand_t->value->uint8;
		persist_write_int(KEY_SHOW_SECOND_HAND, show_second_hand_t->value->uint8);
 	}

	Tuple *color_background_t = dict_find(iter, KEY_COLOR_BACKGROUND);
	if(color_background_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Background color %lu", color_background_t->value->int32);
 		gcolor_background = GColorFromHEX(color_background_t->value->int32);
		window_set_background_color(window, gcolor_background);
		persist_write_int(KEY_COLOR_BACKGROUND, color_background_t->value->int32);
 	}

	Tuple *color_label_t = dict_find(iter, KEY_COLOR_LABEL);
	if(color_label_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Label color %lu", color_label_t->value->int32);
 		gcolor_numbers = GColorFromHEX(color_label_t->value->int32);
		persist_write_int(KEY_COLOR_LABEL, color_label_t->value->int32);
 	}

	Tuple *color_hour_marks_t = dict_find(iter, KEY_COLOR_HOUR_MARKS);
	if(color_hour_marks_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Hour mark color %lu", color_hour_marks_t->value->int32);
 		gcolor_hour_marks = GColorFromHEX(color_hour_marks_t->value->int32);
		persist_write_int(KEY_COLOR_HOUR_MARKS, color_hour_marks_t->value->int32);
 	}

	Tuple *color_minute_marks_t = dict_find(iter, KEY_COLOR_MINUTE_MARKS);
	if(color_minute_marks_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Minute mark color %lu", color_minute_marks_t->value->int32);
 		gcolor_minute_marks = GColorFromHEX(color_minute_marks_t->value->int32);
		persist_write_int(KEY_COLOR_MINUTE_MARKS, color_minute_marks_t->value->int32);
 	}

	Tuple *color_hour_hand_t = dict_find(iter, KEY_COLOR_HOUR_HAND);
	if(color_hour_hand_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Hour hand color %lu", color_hour_hand_t->value->int32);
 		gcolor_hour_hand = GColorFromHEX(color_hour_hand_t->value->int32);
		persist_write_int(KEY_COLOR_HOUR_HAND, color_hour_hand_t->value->int32);
 	}

	Tuple *color_minute_hand_t = dict_find(iter, KEY_COLOR_MINUTE_HAND);
	if(color_minute_hand_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Minute hand color %lu", color_minute_hand_t->value->int32);
 		gcolor_minute_hand = GColorFromHEX(color_minute_hand_t->value->int32);
		persist_write_int(KEY_COLOR_MINUTE_HAND, color_minute_hand_t->value->int32);
 	}

	Tuple *color_second_hand_t = dict_find(iter, KEY_COLOR_SECOND_HAND);
	if(color_second_hand_t) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Second hand color %lu", color_second_hand_t->value->int32);
 		gcolor_second_hand = GColorFromHEX(color_second_hand_t->value->int32);
		persist_write_int(KEY_COLOR_SECOND_HAND, color_second_hand_t->value->int32);
 	}
}

static bool color_hour_marks(GDrawCommand *command, uint32_t index, void *context) {
  gdraw_command_set_stroke_color(command, gcolor_hour_marks);
  return true;
}

static bool color_minute_marks(GDrawCommand *command, uint32_t index, void *context) {
  gdraw_command_set_stroke_color(command, gcolor_minute_marks);
  return true;
}


static int32_t get_angle_for_hour(int hour) {
  // Progress through 12 hours, out of 360 degrees
  return (hour * 360) / 12;
}

static int32_t get_angle_for_minute(int hour) {
  // Progress through 60 miunutes, out of 360 degrees
  return (hour * 360) / 60;
}


static void bg_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);
	GRect frame = grect_inset(bounds, GEdgeInsets(4 * INSET));
	GRect inner_hour_frame = grect_inset(bounds, GEdgeInsets((4 * INSET) + 8));
	GRect inner_minute_frame = grect_inset(bounds, GEdgeInsets((4 * INSET) + 6));
	
	graphics_context_set_stroke_color(ctx, gcolor_hour_marks);
	graphics_context_set_stroke_width(ctx, 3);

	// Hours marks
	for(int i = 0; i < 12; i++) {
		int hour_angle = get_angle_for_hour(i);
		GPoint p0 = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));
		GPoint p1 = gpoint_from_polar(inner_hour_frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));
		graphics_draw_line(ctx, p0, p1);
	}
	
	// Minute Marks
	graphics_context_set_stroke_color(ctx, gcolor_minute_marks);
	graphics_context_set_stroke_width(ctx, 1);
	for(int i = 0; i < 60; i++) {
		if (i % 5) {
			int minute_angle = get_angle_for_minute(i);
			GPoint p0 = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle));
			GPoint p1 = gpoint_from_polar(inner_minute_frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle));
			graphics_draw_line(ctx, p0, p1);
		}
	}
	
	// numbers
	if (b_show_numbers) {
		graphics_context_set_text_color(ctx, gcolor_numbers);
		
#ifdef PBL_RECT
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
#else
		graphics_draw_text(ctx, "12", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(80, 10, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, "1", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(107, 20, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		graphics_draw_text(ctx, "2", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(130, 43, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		graphics_draw_text(ctx, "3", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(140, 74, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		graphics_draw_text(ctx, "4", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(130, 106, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		graphics_draw_text(ctx, "5", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(107, 126, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
		graphics_draw_text(ctx, "6", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(81, 136, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, "7", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(53, 124, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, "8", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(29, 106, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, "9", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(20, 74, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, "10", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(28, 42, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, "11", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(50, 22, 20, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
#endif
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

	// date
	if (b_show_date) {
		graphics_context_set_text_color(ctx, gcolor_numbers);
		int offset = !b_show_numbers * 10;
#ifdef PBL_RECT
		graphics_draw_text(ctx, s_date_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(80, 75, 40 + offset, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#else
		graphics_draw_text(ctx, s_date_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(100, 78, 45 + offset, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#endif
	}

	// temperature
	if (b_show_temperature) {
		graphics_context_set_text_color(ctx, gcolor_numbers);
		int offset = !b_show_numbers * 10;
#ifdef PBL_RECT
		graphics_draw_text(ctx, s_temp_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(27 - offset, 75, 40 + offset, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#else
		graphics_draw_text(ctx, s_temp_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(40 - offset, 78, 40 + offset, 14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#endif
	}

	// minute hand
	graphics_context_set_fill_color(ctx, gcolor_minute_hand);
	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	gpath_draw_filled(ctx, s_minute_arrow);

	// hour hand
	graphics_context_set_fill_color(ctx, gcolor_hour_hand);
	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	gpath_draw_filled(ctx, s_hour_arrow);

	// second hand
	if (b_show_second_hand) {
		graphics_context_set_stroke_color(ctx, gcolor_second_hand);
		graphics_draw_line(ctx, second_hand, center);
		graphics_draw_line(ctx, second_hand_tail, center);
	}

	// dot in the middle
	graphics_context_set_fill_color(ctx, gcolor_second_hand);
	graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);
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

	window_set_background_color(window, gcolor_background);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);

	load_persisted_values();
}

static void window_unload(Window *window) {
	layer_destroy(s_simple_bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_day_label);
	text_layer_destroy(s_num_label);

	layer_destroy(s_hands_layer);
}

static void init() {	
	
	// Default colors
	gcolor_background = GColorBlack;
	gcolor_minute_hand = GColorWhite;
	
	#ifdef PBL_COLOR
		gcolor_hour_marks = GColorLightGray;
		gcolor_minute_marks = GColorDarkGray;
		gcolor_numbers = GColorLightGray;
		gcolor_hour_hand = GColorRed;
		gcolor_second_hand = GColorRed;
	#else
		gcolor_hour_marks = GColorWhite;
		gcolor_minute_marks = GColorWhite;
		gcolor_numbers = GColorWhite;
		gcolor_hour_hand = GColorWhite;
		gcolor_second_hand = GColorWhite;
	#endif

	b_show_numbers = true;
	b_show_second_hand = true;
	b_show_date = true;
	b_show_temperature = true;

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

	app_message_register_inbox_received(inbox_received_handler);
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
