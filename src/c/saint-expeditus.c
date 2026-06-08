#include <pebble.h>

// --- Layers & resources ---
static Window *s_window;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_background_layer;
static TextLayer *s_time_layer;  // displays HH:MM (12-hour, no AM/PM)
static TextLayer *s_date_layer;  // displays day of month (DD), aligned with "HODIE" label on background
static GFont s_font_time;        // Zeus-Land 24pt
static GFont s_font_date;        // Zeus-Land 14pt

// Buffers sized for their exact max output + null terminator
static char s_time_buf[6];  // "HH:MM\0"
static char s_date_buf[3];  // "DD\0"

// --- Time & date update ---

static void update_time(struct tm *t) {
    // Convert 24-hour to 12-hour; midnight/noon edge case: 0 -> 12
    int hour = t->tm_hour % 12;
    if (hour == 0) hour = 12;
    snprintf(s_time_buf, sizeof(s_time_buf), "%02d:%02d", hour, t->tm_min);
    text_layer_set_text(s_time_layer, s_time_buf);
}

static void update_date(struct tm *t) {
    snprintf(s_date_buf, sizeof(s_date_buf), "%02d", t->tm_mday);
    text_layer_set_text(s_date_layer, s_date_buf);
}

// Fires every minute; updates both time and date
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
    update_date(tick_time);
}

// --- Window lifecycle ---

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);

    // Background: saint-expeditus_v3.png (200x228, designed for Emery/Pebble Time 2)
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    s_background_layer = bitmap_layer_create(layer_get_bounds(root));
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    bitmap_layer_set_compositing_mode(s_background_layer, GCompOpAssign);
    layer_add_child(root, bitmap_layer_get_layer(s_background_layer));

    s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ZEUS_LAND_24));
    s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ZEUS_LAND_14));

    // Time layer — positioned over the time box in the v3 background layout
    s_time_layer = text_layer_create(GRect(10, 41, 56, 31));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_font(s_time_layer, s_font_time);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_layer));

    // Date layer — positioned over the "HODIE" box in the v3 background layout
    // Note: y=174 places this below the visible area on screens smaller than Emery (200x228)
    s_date_layer = text_layer_create(GRect(25, 174, 27, 27));
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorBlack);
    text_layer_set_font(s_date_layer, s_font_date);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_date_layer));

    // Populate layers immediately so there's no blank display on load
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    update_time(t);
    update_date(t);
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    fonts_unload_custom_font(s_font_time);
    fonts_unload_custom_font(s_font_date);
    bitmap_layer_destroy(s_background_layer);
    gbitmap_destroy(s_background_bitmap);
}

// --- App lifecycle ---

static void init(void) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
