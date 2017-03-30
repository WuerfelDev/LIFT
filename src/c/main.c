#include "pebble.h"

#define PANELH  50

static Window *window;
static TextLayer *timeLayer;

/*
-- TODO --
-show icon after disconnect for some time (not important)
-animations (quickview & full hour) - what about hour-change while quickview is on!
                                    - how does it work when the location is overwritten by handle_tick?

-far future:
-settings with negative colors on monochrome/BW models
-settings to disable vibration on disconnect (-and icon)

*/


static void handle_tick(struct tm *currentTime, TimeUnits units_changed) {
  Layer *timeLayer_new = text_layer_get_layer(timeLayer);
  GRect bounds = layer_get_bounds(timeLayer_new);
  
  static char timeText[] = "LI:FT"; //00:00
  strftime(timeText, sizeof(timeText), clock_is_24h_style() ? "%H:%M" : "%I:%M", currentTime);
  text_layer_set_text(timeLayer, timeText);
  bounds.origin.y = currentTime->tm_min*(layer_get_unobstructed_bounds(window_get_root_layer(window)).size.h-PANELH)/60;

  //bounds.origin.y = currentTime->tm_min*(layer_get_bounds(window_get_root_layer(window)).size.h-PANELH)/60;
  layer_set_frame(timeLayer_new, bounds);
  //layer_mark_dirty(timeLayer_new); // Not needed, works fine without
}




static void bluetooth_callback(bool connected) {
  if(!connected) {
    vibes_double_pulse();
  }
}

static void unobstructed_will(GRect final_unobstructed_screen_area, void *context) {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
}

static void unobstructed_did(void *context) {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
}
static void unobstructed_change(AnimationProgress progress, void *context) {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);
}



static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  timeLayer = text_layer_create(GRect(0, 0, layer_get_bounds(window_layer).size.w, PANELH ));
  #ifdef PBL_COLOR
  text_layer_set_text_color(timeLayer, GColorWhite); // GColorWhite
  text_layer_set_background_color(timeLayer, GColorRed); // GColorBlack
  #else
  text_layer_set_text_color(timeLayer, GColorBlack); // GColorWhite
  text_layer_set_background_color(timeLayer, GColorWhite); // GColorBlack
  #endif
  text_layer_set_font(timeLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(timeLayer, GTextAlignmentCenter);
  

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, MINUTE_UNIT);

  layer_add_child(window_layer, text_layer_get_layer(timeLayer));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  
  UnobstructedAreaHandlers handlers = {
    .will_change = unobstructed_will,
    .change = unobstructed_change,
    .did_change = unobstructed_did
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}


static void main_window_unload(Window *window) {
  connection_service_unsubscribe();
  unobstructed_area_service_unsubscribe();
  text_layer_destroy(timeLayer);
}



static void app_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  connection_service_subscribe((ConnectionHandlers) {
  .pebble_app_connection_handler = bluetooth_callback
});
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, true);  // Animated
}

static void app_deinit(void) {
  window_destroy(window);
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
