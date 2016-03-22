#include "pebble.h"


static Window *window;

static TextLayer *timeLayer;



static void handle_second_tick(struct tm *currentTime, TimeUnits units_changed) {
  static char timeText[] = "Li:ft"; //00:00
  strftime(timeText, sizeof(timeText), clock_is_24h_style() ? "%H:%M" : "%I:%M", currentTime);
  text_layer_set_text(timeLayer, timeText);
  int minuts = currentTime->tm_min;
  int secs = currentTime->tm_sec;
  Layer *timeLayer_layer = text_layer_get_layer(timeLayer);
  GRect bounds = layer_get_bounds(timeLayer_layer);
  
  // printf("Minuts: %i",minuts);
  // printf("Calc: %f", ((minuts+(0.01*secs))*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/(60))+(minuts/59));
  bounds.origin.y = (minuts+(secs/59))*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/60;//+(minuts/59);//60-1 //SECONDS_PER_HOUR-1(?)
  //printf("Calc: %i", secs*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/(59));
  //bounds.origin.y = secs*(layer_get_bounds(window_get_root_layer(window)).size.h-bounds.size.h)/(59);//60-1 //SECONDS_PER_HOUR-1(?)
  layer_set_frame(timeLayer_layer, bounds);
  layer_mark_dirty(timeLayer_layer);
}




static void app_init(void) {
  window = window_create();
  Layer *window_layer = window_get_root_layer(window);
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack); // GColorWhite
  timeLayer = text_layer_create(GRect(0, 0, layer_get_bounds(window_layer).size.w, 50));
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
  handle_second_tick(tick_time, SECOND_UNIT);

  layer_add_child(window_layer, text_layer_get_layer(timeLayer));

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  

}

static void app_deinit(void) {
  text_layer_destroy(timeLayer);
  window_destroy(window);
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
