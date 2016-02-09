#include <pebble.h>

#define WIDTH 144
#define HEIGHT 168
#define XCENTER 72
#define YCENTER 84
#define EXTERNAL_RADIUS (XCENTER-1)
#define CIRCLE_THICKNESS 15

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_utc_time_layer;
static TextLayer *s_date_layer;

static GColor color_main;
static GColor color_accent;
static GColor color_warn;
static GColor color_error;

static Layer *s_window_root_layer;
static Layer *s_canvas;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_time_font;

static int s_hours;
static int s_minutes;

static int32_t get_increment_angle_for_minute(int minute) {
  // Progress through 60 minutes, out of 360 degrees
  return ((minute - minute % 5) * 360) / 60;
}

static int32_t get_angle_for_minutes(int minute) {
  return (minute * 360) / 60;
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  struct tm *gm_time = gmtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  if(clock_is_24h_style()) {
    // Use 24 hour format
    strftime(s_buffer, sizeof(s_buffer), "%H", tick_time);
    text_layer_set_text(s_time_layer,s_buffer);
  } else {
    // Use 12 hour format
    strftime(s_buffer, sizeof(s_buffer), "%I", tick_time);
    text_layer_set_text(s_time_layer,s_buffer+(('0' == s_buffer[0])?1:0));  
  }

  static char s_utc_buffer[8];
  strftime(s_utc_buffer, sizeof(s_utc_buffer), "%H", gm_time);
  text_layer_set_text(s_utc_time_layer, s_utc_buffer);
  
  static char s_date_buffer[10];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%x", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
  s_hours = tick_time->tm_hour;
  s_minutes = tick_time->tm_min;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // 12 hours only, with a minimum size
  s_hours -= (s_hours > 12) ? 12 : 0;
  GRect frame = grect_inset(bounds, GEdgeInsets(1));
  GRect dots_frame = grect_inset(bounds, GEdgeInsets(3));
  
  for(int i = 1; i <= 60; i++) {
    int minutes_angle = get_angle_for_minutes(i);
    GPoint pos = gpoint_from_polar(dots_frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minutes_angle));
    GColor color;
    if (i <= (s_minutes - s_minutes % 5) || i > s_minutes) {
      color = GColorClear;
    } else {
      color = color_accent;
    }
    //TODO: change the colors based on 1-4
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_circle(ctx, pos, 2);
  }

  // Minutes are expanding circle arc
  int minute_angle = get_increment_angle_for_minute(s_minutes);
  graphics_context_set_fill_color(ctx, color_main);
  graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 6, 0, DEG_TO_TRIGANGLE(minute_angle));
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void battery_handler(BatteryChargeState charge_state) {
  if (charge_state.is_charging) {
    text_layer_set_text_color(s_time_layer, color_accent);
  } else if (charge_state.charge_percent > 20) {
    text_layer_set_text_color(s_time_layer, GColorWhite);
  } else if (charge_state.charge_percent <= 10) {
    text_layer_set_text_color(s_time_layer, color_error);
  } else if (charge_state.charge_percent <= 20) {
    text_layer_set_text_color(s_time_layer, color_warn);
  }
}

static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    text_layer_set_text_color(s_date_layer, color_main);
  } else {
    vibes_double_pulse();
    text_layer_set_text_color(s_date_layer, color_error);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  s_window_root_layer = window_get_root_layer(window);

  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
  GRect bounds = layer_get_bounds(s_window_root_layer);
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(s_window_root_layer, bitmap_layer_get_layer(s_background_layer));

  //primary hours indicator
  s_time_layer = text_layer_create(GRect(0, 30, bounds.size.w, 46));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "0");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // gm time
  s_utc_time_layer = text_layer_create(GRect(0, 96, bounds.size.w, 24));
  text_layer_set_background_color(s_utc_time_layer, GColorClear);
  text_layer_set_text_color(s_utc_time_layer, color_accent);
  text_layer_set_font(s_utc_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_utc_time_layer, "00:00");
  text_layer_set_text_alignment(s_utc_time_layer, GTextAlignmentCenter);
  
  // day of the week
  s_date_layer = text_layer_create(GRect(0, 76, bounds.size.w, 18));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, color_main);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_date_layer, "00:00");
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Create GFont
  //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // Apply to TextLayer
  //text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_root_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(s_window_root_layer, text_layer_get_layer(s_utc_time_layer));
  layer_add_child(s_window_root_layer, text_layer_get_layer(s_date_layer));
  
  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, layer_update_proc);
  layer_add_child(s_window_root_layer, s_canvas);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  color_main = GColorYellow;
  color_accent = GColorRed;
  color_warn = GColorBabyBlueEyes;
  color_error = GColorCyan;

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bt_handler
  });

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}