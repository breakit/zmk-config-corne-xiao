/*
 * Full custom status screen, labels only (no canvas yet).
 * Layer, output, battery (central), WPM number, modifiers — as plain LVGL
 * labels in a vertical 128x32 stack. Exercises all central state reads.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/endpoints_types.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>
#include <zmk/wpm.h>
#include <zmk/split/central.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/wpm_state_changed.h>

#include <dt-bindings/zmk/modifiers.h>

/* nice!oled-inspired layout for the 128x32 SSD1306.
 * Two horizontal bands, each with a boxed icon + text region:
 *   Top   : [ USBBT ] [ layer ]          [ mods ]   y 0..15
 *   Bottom: [ BATT% ] [ WPM num ] [sparkline ]      y 16..31
 */
#define ICON_W 22
#define ICON_H 15
#define ICONX 2
#define ICONY_TOP 1
#define ICONY_BOT 17

#define NAME_X 28
#define NAME_Y_TOP 4

#define MOD_X 88

#define BATT_TEXT_X 28
#define BATT_TEXT_Y_BOT 19
#define WPM_NUM_X 54
#define WPM_NUM_Y_BOT 17
#define GRAPH_X 96
#define GRAPH_Y (17 + 1)
#define GRAPH_W 30
#define GRAPH_H 14
#define WPM_HISTORY_LEN 24

static lv_obj_t *refs_output_icon; /* boxed output icon */
static lv_obj_t *refs_output_label;
static lv_obj_t *refs_layer;
static lv_obj_t *refs_wpm_number;
static lv_obj_t *refs_wpm_graph;
static lv_obj_t *refs_battery_icon; /* boxed battery icon */
static lv_obj_t *refs_battery_icon_text;
static lv_obj_t *refs_battery_label;
static lv_obj_t *refs_modifiers;

static uint8_t wpm_history[WPM_HISTORY_LEN];
static uint8_t wpm_head;
static lv_point_t graph_pts[WPM_HISTORY_LEN];

static void wpm_history_push(uint8_t wpm) {
    wpm_head = (wpm_head + 1) % WPM_HISTORY_LEN;
    wpm_history[wpm_head] = wpm;
}

/* Recompute the sparkline point array from wpm_history[]. */
static void update_graph_points(void) {
    for (int i = 0; i < WPM_HISTORY_LEN; i++) {
        int idx =
            ((int)wpm_head - (WPM_HISTORY_LEN - 1 - i) + 2 * WPM_HISTORY_LEN) % WPM_HISTORY_LEN;
        uint8_t v = wpm_history[idx];
        int y = (GRAPH_H - 1) - (v > 100 ? 100 : v) * (GRAPH_H - 1) / 100;
        int x = i * (GRAPH_W - 1) / (WPM_HISTORY_LEN - 1);
        graph_pts[i].x = x;
        graph_pts[i].y = y;
    }
}

static void draw_wpm_graph(void) {
    if (refs_wpm_graph == NULL)
        return;
    update_graph_points();
    lv_line_set_points(refs_wpm_graph, graph_pts, WPM_HISTORY_LEN);
}

struct status_state {
    struct zmk_endpoint_instance selected_endpoint;
    bool ble_connected;
    bool ble_bonded;
    uint8_t battery;
    bool charging;
    uint8_t peripheral_battery;
    uint8_t layer_index;
    const char *layer_label;
    uint8_t wpm;
    uint8_t modifiers;
};

static struct status_state status_state_get(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *batt_ev = as_zmk_battery_state_changed(eh);
    uint8_t layer = zmk_keymap_highest_layer_active();

    struct status_state s = {
        .selected_endpoint = zmk_endpoint_get_selected(),
        .ble_connected = zmk_ble_active_profile_is_connected(),
        .ble_bonded = !zmk_ble_active_profile_is_open(),
        .battery = (batt_ev != NULL) ? batt_ev->state_of_charge : zmk_battery_state_of_charge(),
        .charging = zmk_usb_is_powered(),
        .peripheral_battery = 0,
        .layer_index = layer,
        .layer_label = zmk_keymap_layer_name(layer),
        .wpm = zmk_wpm_get_state(),
        .modifiers = (uint8_t)zmk_hid_get_explicit_mods(),
    };
    return s;
}

static void redraw_cb(struct status_state *s) {
    const char *out = " ";
    switch (s->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        out = LV_SYMBOL_USB;
        break;
    case ZMK_TRANSPORT_BLE:
        out = s->ble_bonded ? (s->ble_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_BLUETOOTH)
                            : LV_SYMBOL_BELL;
        break;
    default:
        out = " ";
        break;
    }
    lv_label_set_text(refs_output_label, out);

    if (s->layer_label != NULL) {
        lv_label_set_text(refs_layer, s->layer_label);
    } else {
        char text[16] = {};
        snprintf(text, sizeof(text), "L%d", s->layer_index);
        lv_label_set_text(refs_layer, text);
    }

    /* Battery: boxed icon shows a charge/battery glyph + level text. */
    const char *batt_glyph = LV_SYMBOL_BATTERY_EMPTY;
    if (s->battery > 95) {
        batt_glyph = LV_SYMBOL_BATTERY_FULL;
    } else if (s->battery > 65) {
        batt_glyph = LV_SYMBOL_BATTERY_3;
    } else if (s->battery > 35) {
        batt_glyph = LV_SYMBOL_BATTERY_2;
    } else if (s->battery > 5) {
        batt_glyph = LV_SYMBOL_BATTERY_1;
    }
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    if (s->charging) {
        batt_glyph = LV_SYMBOL_CHARGE;
    }
#endif
    lv_label_set_text(refs_battery_icon_text, batt_glyph);

    char text[16] = {};
    bool show_periph = s->peripheral_battery > 0 && s->peripheral_battery <= 100;
    if (show_periph && IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)) {
        snprintf(text, sizeof(text), "%d/%d", s->battery, s->peripheral_battery);
    } else {
        snprintf(text, sizeof(text), "%d", s->battery);
    }
    lv_label_set_text(refs_battery_label, text);

    char mods[8] = {};
    if (s->modifiers & MOD_LCTL)
        strcat(mods, "C");
    if (s->modifiers & MOD_LSFT)
        strcat(mods, "S");
    if (s->modifiers & MOD_LALT)
        strcat(mods, "A");
    if (s->modifiers & MOD_LGUI)
        strcat(mods, "G");
    lv_label_set_text(refs_modifiers, mods);
}

static void output_update_cb(struct status_state s) { redraw_cb(&s); }
static void battery_update_cb(struct status_state s) { redraw_cb(&s); }
static void layer_update_cb(struct status_state s) { redraw_cb(&s); }
static void modifiers_update_cb(struct status_state s) { redraw_cb(&s); }
static void wpm_update_cb(struct status_state s) {
    wpm_history_push(s.wpm);
    char text[8] = {};
    snprintf(text, sizeof(text), "%d", s.wpm);
    lv_label_set_text(refs_wpm_number, text);
    draw_wpm_graph();
}

/* Peripheral (right) battery relayed over BLE, shown on the central screen. */
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
static void peripheral_batt_update_cb(struct status_state s) { redraw_cb(&s); }

static struct status_state peripheral_batt_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);
    struct status_state s = status_state_get(NULL);
    s.peripheral_battery = (ev != NULL) ? ev->state_of_charge : 0;
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_periph_batt, struct status_state,
                            peripheral_batt_update_cb, peripheral_batt_get_state)
#endif

ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_output, struct status_state, output_update_cb,
                            status_state_get)
ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_battery, struct status_state, battery_update_cb,
                            status_state_get)
ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_layer, struct status_state, layer_update_cb,
                            status_state_get)
ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_mods, struct status_state, modifiers_update_cb,
                            status_state_get)
ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_wpm, struct status_state, wpm_update_cb,
                            status_state_get)

ZMK_SUBSCRIPTION(widget_nice_oled_output, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_output, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_battery, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_layer, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_mods, zmk_modifiers_state_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_wpm, zmk_wpm_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
ZMK_SUBSCRIPTION(widget_nice_oled_periph_batt, zmk_peripheral_battery_state_changed);
#endif
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_nice_oled_output, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_battery, zmk_usb_conn_state_changed);
#endif

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);
    lv_obj_set_style_border_width(screen, 0, 0);

    /* Inverted icon style: white box, black glyph. */
    static lv_style_t st_icon;
    lv_style_init(&st_icon);
    lv_style_set_bg_color(&st_icon, lv_color_white());
    lv_style_set_bg_opa(&st_icon, LV_OPA_COVER);
    lv_style_set_text_color(&st_icon, lv_color_black());
    lv_style_set_pad_all(&st_icon, 2);
    lv_style_set_radius(&st_icon, 0);

    static lv_style_t st_name;
    lv_style_init(&st_name);
    lv_style_set_text_font(&st_name, &lv_font_montserrat_16);

    /* ---- Top band: output icon box + layer name + mods ---- */
    refs_output_icon = lv_obj_create(screen);
    lv_obj_add_style(refs_output_icon, &st_icon, 0);
    lv_obj_set_size(refs_output_icon, ICON_W, ICON_H);
    lv_obj_set_pos(refs_output_icon, ICONX, ICONY_TOP);
    refs_output_label = lv_label_create(refs_output_icon);
    lv_label_set_text(refs_output_label, " ");
    lv_obj_center(refs_output_label);

    refs_layer = lv_label_create(screen);
    lv_obj_add_style(refs_layer, &st_name, 0);
    lv_label_set_text(refs_layer, "-");
    lv_obj_set_pos(refs_layer, NAME_X, NAME_Y_TOP);

    refs_modifiers = lv_label_create(screen);
    lv_label_set_text(refs_modifiers, "");
    lv_obj_set_pos(refs_modifiers, MOD_X, NAME_Y_TOP + 3);

    /* ---- Bottom band: battery icon box (+ level) + WPM number + sparkline ---- */
    refs_battery_icon = lv_obj_create(screen);
    lv_obj_add_style(refs_battery_icon, &st_icon, 0);
    lv_obj_set_size(refs_battery_icon, ICON_W, ICON_H);
    lv_obj_set_pos(refs_battery_icon, ICONX, ICONY_BOT);
    refs_battery_icon_text = lv_label_create(refs_battery_icon);
    lv_label_set_text(refs_battery_icon_text, LV_SYMBOL_BATTERY_3);
    lv_obj_center(refs_battery_icon_text);

    refs_battery_label = lv_label_create(screen);
    lv_label_set_text(refs_battery_label, "0");
    lv_obj_set_pos(refs_battery_label, BATT_TEXT_X, BATT_TEXT_Y_BOT);

    refs_wpm_number = lv_label_create(screen);
    lv_obj_add_style(refs_wpm_number, &st_name, 0);
    lv_label_set_text(refs_wpm_number, "0");
    lv_obj_set_pos(refs_wpm_number, WPM_NUM_X, WPM_NUM_Y_BOT);

    refs_wpm_graph = lv_line_create(screen);
    lv_line_set_points(refs_wpm_graph, graph_pts, WPM_HISTORY_LEN);
    lv_obj_set_pos(refs_wpm_graph, GRAPH_X, GRAPH_Y);

    widget_nice_oled_output_init();
    widget_nice_oled_battery_init();
    widget_nice_oled_layer_init();
    widget_nice_oled_mods_init();
    widget_nice_oled_wpm_init();
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    widget_nice_oled_periph_batt_init();
#endif

    return screen;
}
