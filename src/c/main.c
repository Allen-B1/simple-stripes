/* Copyright (c) Allen Bao
 * Licensed under the GNU GPL */

#include <pebble.h>

static Window* s_main_window;
static TextLayer *s_text_layer, *s_date_layer, *s_year_layer, *s_battery_layer;
static int s_bat_level;
GRect bounds;

static void update_bat(BatteryChargeState state) {
  static char buf[5];
  s_bat_level = state.charge_percent;
  snprintf(buf, 5, "%d%%", s_bat_level);
  text_layer_set_text(s_battery_layer, buf);
  GSize size = {.h=18};
  size.w = (bounds.size.w * state.charge_percent) / 100;
  text_layer_set_size(s_battery_layer, size);
}

static void update_time(struct tm* tick_time) {
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_text_layer, s_buffer);

  static char s_year_buffer[5];
  snprintf(s_year_buffer, sizeof(s_year_buffer), "%d", tick_time->tm_year + 1900);
  text_layer_set_text(s_year_layer, s_year_buffer);
  
  static char date_buffer[24];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  
  text_layer_set_text(s_date_layer, date_buffer);
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorWhite);
  bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_text_layer = text_layer_create(
      GRect(0, bounds.size.h / 2 - 32, bounds.size.w, 42));

  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorBlack);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  s_date_layer = text_layer_create(GRect(0, 0, bounds.size.w, 36));     
  text_layer_set_background_color(s_date_layer, GColorLightGray);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Mon 4 Jul");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  s_battery_layer = text_layer_create(GRect(0, bounds.size.h - 18, bounds.size.w, 18));
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text(s_battery_layer, "100%");
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);

  s_year_layer = text_layer_create(GRect(0, bounds.size.h - 44, bounds.size.w, 44));    
  text_layer_set_background_color(s_year_layer, GColorLightGray);
  text_layer_set_text_color(s_year_layer, GColorWhite);
  text_layer_set_text(s_year_layer, "1900");
  text_layer_set_font(s_year_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_year_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_year_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_time(tick_time);
  update_bat(battery_state_service_peek());
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}


static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_year_layer);
}

static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(update_bat);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
