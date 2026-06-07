#include <pebble.h>

static Window *s_window;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_background_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_ampm_layer;
static GFont s_font_26;
static GFont s_font_14;

static char s_hour_buf[3];
static char s_minute_buf[3];
static char s_ampm_buf[3];

static void update_time(struct tm *t) {
    int hour = t->tm_hour % 12;
    if (hour == 0) hour = 12;
    snprintf(s_hour_buf, sizeof(s_hour_buf), "%02d", hour);
    snprintf(s_minute_buf, sizeof(s_minute_buf), "%02d", t->tm_min);
    strftime(s_ampm_buf, sizeof(s_ampm_buf), "%p", t);

    text_layer_set_text(s_hour_layer, s_hour_buf);
    text_layer_set_text(s_minute_layer, s_minute_buf);
    text_layer_set_text(s_ampm_layer, s_ampm_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
}

static void make_text_layer(TextLayer **out, GRect frame, GFont font) {
    *out = text_layer_create(frame);
    text_layer_set_background_color(*out, GColorClear);
    text_layer_set_text_color(*out, GColorRed);
    text_layer_set_font(*out, font);
    text_layer_set_text_alignment(*out, GTextAlignmentLeft);
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);

    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    s_background_layer = bitmap_layer_create(layer_get_bounds(root));
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    bitmap_layer_set_compositing_mode(s_background_layer, GCompOpAssign);
    layer_add_child(root, bitmap_layer_get_layer(s_background_layer));

    s_font_26 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SAINT_MARTYR_EXPAND_26));
    s_font_14 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SAINT_MARTYR_EXPAND_14));

    make_text_layer(&s_hour_layer,   GRect(12, 11, 55, 30), s_font_26);
    make_text_layer(&s_minute_layer, GRect(12, 41, 55, 30), s_font_26);
    make_text_layer(&s_ampm_layer,   GRect(12, 71, 35, 18), s_font_14);

    layer_add_child(root, text_layer_get_layer(s_hour_layer));
    layer_add_child(root, text_layer_get_layer(s_minute_layer));
    layer_add_child(root, text_layer_get_layer(s_ampm_layer));

    time_t now = time(NULL);
    update_time(localtime(&now));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_hour_layer);
    text_layer_destroy(s_minute_layer);
    text_layer_destroy(s_ampm_layer);
    fonts_unload_custom_font(s_font_26);
    fonts_unload_custom_font(s_font_14);
    bitmap_layer_destroy(s_background_layer);
    gbitmap_destroy(s_background_bitmap);
}

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
