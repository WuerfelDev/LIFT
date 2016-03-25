#include "pebble.h"

#define PANELH  50

static Window *window;
static TextLayer *timeLayer;

static void handle_tick(struct tm *currentTime, TimeUnits units_changed) {
if( (units_changed & MINUTE_UNIT) != 0 ) {
  static char timeText[] = "LI:FT"; //00:00
  strftime(timeText, sizeof(timeText), clock_is_24h_style() ? "%H:%M" : "%I:%M", currentTime);
  text_layer_set_text(timeLayer, timeText);
}
  Layer *timeLayer_layer = text_layer_get_layer(timeLayer);
  
  GRect bounds = layer_get_bounds(timeLayer_layer);
  int calct = (currentTime->tm_min*60+currentTime->tm_sec)*(layer_get_bounds(window_get_root_layer(window)).size.h-PANELH)/3600;
  if(bounds.origin.y != calct){
    //bounds.origin.y = (minuts+(secs/59))*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/60;  //OLD: UPDATES EVER MINUTE FOR MULTIPLE PX
    bounds.origin.y = calct;
    //printf("A: %i", (minuts+(secs/59))*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/60);
    //printf("N: %i", (minuts*60+secs)*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/3600);
  
    layer_set_frame(timeLayer_layer, bounds);
    layer_mark_dirty(timeLayer_layer);
  }
}



static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  timeLayer = text_layer_create(GRect(0, 0, layer_get_bounds(window_layer).size.w, PANELH));
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

  tick_timer_service_subscribe(SECOND_UNIT | MINUTE_UNIT, handle_tick);
}


static void main_window_unload(Window *window) {
  text_layer_destroy(timeLayer);
}



static void app_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_set_background_color(window, GColorBlack); // GColorWhite
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
