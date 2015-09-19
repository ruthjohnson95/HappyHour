#include "pebble.h"

#define REPEAT_INTERVAL_MS 50
  
#define TEA_TEXT_GAP 14
  
// This is a custom defined key for saving our count field
#define NUM_DRINKS_PKEY 1

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0

static Window *s_main_window, *s_height_menu_window, *s_weight_menu_window, *s_gender_menu_window; 
static MenuLayer *s_menu_layer;
static ActionBarLayer *s_action_bar;
static TextLayer *s_header_layer, *s_body_layer, *s_label_layer, 
/* from tea tutorial */
*s_error_text_layer, *s_tea_text_layer, *s_countdown_text_layer, *s_cancel_text_layer;
static GBitmap *s_icon_plus, *s_icon_minus;

static int s_num_drinks = NUM_DRINKS_DEFAULT;

/* variables for calculating BAC */
int m_height, m_weight, m_gender;
int height[] = {165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182};
int weight[] = {65, 70, 75, 80, 85, 90, 95, 100};
int gender[] = {0, 1};

static char s_tea_text[32];

static void height_select_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index, void *callback_context) {
  // If we were displaying s_error_text_layer, remove it and return
  if (!layer_get_hidden(text_layer_get_layer(s_error_text_layer))) {
    layer_set_hidden(text_layer_get_layer(s_error_text_layer), true);
    return;
  }

  m_height = height[cell_index->row];
  APP_LOG(APP_LOG_LEVEL_DEBUG, "height is now %d", m_height); 
  
  // Switch to s_weight_main_window
  window_stack_push(s_weight_menu_window, false);
}

static uint16_t height_get_sections_count_callback(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context) {
  int count = sizeof(height);
  return count;
}


static void weight_select_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index, void *callback_context) {
  // If we were displaying s_error_text_layer, remove it and return
  if (!layer_get_hidden(text_layer_get_layer(s_error_text_layer))) {
    layer_set_hidden(text_layer_get_layer(s_error_text_layer), true);
    return;
  }

  m_weight = weight[cell_index->row];
  APP_LOG(APP_LOG_LEVEL_DEBUG, "weight is now %d", m_weight); 
  
  // Switch to s_main_window
  window_stack_push(s_main_window, false);
}

static uint16_t weight_get_sections_count_callback(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context) {
  int count = sizeof(weight);
  return count;
}

/* Height Draw Row Handler - draws the menu */
static void height_draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  char* name = "";
  int text_gap_size = TEA_TEXT_GAP - strlen(name);
  int temp_height = height[cell_index->row];

  // Using simple space padding between name and s_tea_text for appearance of edge-alignment
  snprintf(s_tea_text, sizeof(s_tea_text), "%s%*s%d cm", name, text_gap_size, "", temp_height);

  menu_cell_basic_draw(ctx, cell_layer, s_tea_text, NULL, NULL);
}


/* Weight Draw Row Handler - draws the menu */
static void weight_draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  char* name = "";
  int text_gap_size = TEA_TEXT_GAP - strlen(name);
  int temp_weight = weight[cell_index->row];

  // Using simple space padding between name and s_tea_text for appearance of edge-alignment
  snprintf(s_tea_text, sizeof(s_tea_text), "%s%*s%d kg", name, text_gap_size, "", temp_weight);

  menu_cell_basic_draw(ctx, cell_layer, s_tea_text, NULL, NULL);
}

static void update_text() {
  static char s_body_text[18];
  snprintf(s_body_text, sizeof(s_body_text), "%u Platpuses", s_num_drinks);
  text_layer_set_text(s_body_layer, s_body_text);
}

static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_num_drinks++;
  update_text();
}

static void decrement_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_num_drinks <= 0) {
    // Keep the counter at zero
    return;
  }

  s_num_drinks--;
  update_text();
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_INTERVAL_MS, increment_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, decrement_click_handler);
}

/* HEIGHT MENU WINDOW LOAD */

static void height_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = height_get_sections_count_callback,
    .draw_row = height_draw_row_handler,
    .select_click = height_select_callback
  }); 
  menu_layer_set_click_config_onto_window(s_menu_layer,	window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_error_text_layer = text_layer_create((GRect) { .origin = {0, 44}, .size = {bounds.size.w, 60}});
  text_layer_set_text(s_error_text_layer, "Cannot\nschedule");
  text_layer_set_text_alignment(s_error_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_error_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_error_text_layer, GColorWhite);
  text_layer_set_background_color(s_error_text_layer, GColorBlack);
  layer_set_hidden(text_layer_get_layer(s_error_text_layer), true);
  layer_add_child(window_layer, text_layer_get_layer(s_error_text_layer));
}

static void weight_menu_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_error_text_layer);
}

/* WEIGHT MENU WINDOW LOAD */

static void weight_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = weight_get_sections_count_callback,
    .draw_row = weight_draw_row_handler,
    .select_click = weight_select_callback
  }); 
  menu_layer_set_click_config_onto_window(s_menu_layer,	window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_error_text_layer = text_layer_create((GRect) { .origin = {0, 44}, .size = {bounds.size.w, 60}});
  text_layer_set_text(s_error_text_layer, "Cannot\nschedule");
  text_layer_set_text_alignment(s_error_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_error_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_error_text_layer, GColorWhite);
  text_layer_set_background_color(s_error_text_layer, GColorBlack);
  layer_set_hidden(text_layer_get_layer(s_error_text_layer), true);
  layer_add_child(window_layer, text_layer_get_layer(s_error_text_layer));
}

static void height_menu_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_error_text_layer);
}

/* MENU WINDOW LOAD */

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);

  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_plus);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_minus);

  int width = layer_get_frame(window_layer).size.w - ACTION_BAR_WIDTH - 3;

  s_header_layer = text_layer_create(GRect(4, 0, width, 60));
  text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(s_header_layer, GColorClear);
  text_layer_set_text(s_header_layer, "Drink Counter");
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_body_layer = text_layer_create(GRect(4, 44, width, 60));
  text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_body_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_body_layer));

  s_label_layer = text_layer_create(GRect(4, 44 + 28, width, 60));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text(s_label_layer, "of drinks on the wall");
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  update_text();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_body_layer);
  text_layer_destroy(s_label_layer);

  action_bar_layer_destroy(s_action_bar);
}

static void init() {
   
  s_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  s_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);

  // Get the count from persistent storage for use if it exists, otherwise use the default
  s_num_drinks = persist_exists(NUM_DRINKS_PKEY) ? persist_read_int(NUM_DRINKS_PKEY) : NUM_DRINKS_DEFAULT;
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  s_weight_menu_window = window_create();
  window_set_window_handlers(s_weight_menu_window, (WindowHandlers){
    .load = weight_menu_window_load,
    .unload = weight_menu_window_unload,
  });
  window_stack_push(s_weight_menu_window, true);
  
s_height_menu_window = window_create();
  window_set_window_handlers(s_height_menu_window, (WindowHandlers){
    .load = height_menu_window_load,
    .unload = height_menu_window_unload,
  });
  window_stack_push(s_height_menu_window, true);
}

static void deinit() {
  // Save the count into persistent storage on app exit
  persist_write_int(NUM_DRINKS_PKEY, s_num_drinks);
  
  window_destroy(s_main_window);

  gbitmap_destroy(s_icon_plus);
  gbitmap_destroy(s_icon_minus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
