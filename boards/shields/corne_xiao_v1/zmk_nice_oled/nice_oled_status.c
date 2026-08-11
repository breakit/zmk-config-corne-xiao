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

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/wpm_state_changed.h>

#include <dt-bindings/zmk/modifiers.h>

#define LEGEND_X 2
#define VALUE_X 40

#define ROW_LAYER 2
#define ROW_WPM 11
#define ROW_BAT 22

static lv_obj_t *refs_output;
static lv_obj_t *refs_layer;
static lv_obj_t *refs_wpm_number;
static lv_obj_t *refs_battery;
static lv_obj_t *refs_modifiers;

struct status_state {
    struct zmk_endpoint_instance selected_endpoint;
    bool ble_connected;
    bool ble_bonded;
    uint8_t battery;
    bool charging;
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
    lv_label_set_text(refs_output, out);

    if (s->layer_label != NULL) {
        lv_label_set_text(refs_layer, s->layer_label);
    } else {
        char text[16] = {};
        snprintf(text, sizeof(text), "L%d", s->layer_index);
        lv_label_set_text(refs_layer, text);
    }

    char text[16] = {};
    snprintf(text, sizeof(text), "%d%%", s->battery);
    lv_label_set_text(refs_battery, text);

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
    char text[8] = {};
    snprintf(text, sizeof(text), "%d", s.wpm);
    lv_label_set_text(refs_wpm_number, text);
}

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
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_nice_oled_output, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(widget_nice_oled_battery, zmk_usb_conn_state_changed);
#endif

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    static const struct {
        const char *text;
        lv_coord_t x, y;
    } legends[] = {
        {"LAYER", LEGEND_X, ROW_LAYER + 2},
        {"WPM", LEGEND_X, ROW_WPM + 2},
        {"BATT", LEGEND_X, ROW_BAT + 2},
        {"MOD", 90, ROW_BAT + 2},
    };

    for (size_t i = 0; i < ARRAY_SIZE(legends); i++) {
        lv_obj_t *l = lv_label_create(screen);
        lv_label_set_text(l, legends[i].text);
        lv_obj_set_pos(l, legends[i].x, legends[i].y);
    }

    refs_output = lv_label_create(screen);
    lv_label_set_text(refs_output, " ");
    lv_obj_set_pos(refs_output, 100, ROW_LAYER + 2);

    refs_layer = lv_label_create(screen);
    lv_label_set_text(refs_layer, "-");
    lv_obj_set_pos(refs_layer, VALUE_X, ROW_LAYER + 2);

    refs_wpm_number = lv_label_create(screen);
    lv_label_set_text(refs_wpm_number, "0");
    lv_obj_set_pos(refs_wpm_number, VALUE_X, ROW_WPM + 2);

    refs_battery = lv_label_create(screen);
    lv_label_set_text(refs_battery, "-");
    lv_obj_set_pos(refs_battery, VALUE_X, ROW_BAT + 2);

    refs_modifiers = lv_label_create(screen);
    lv_label_set_text(refs_modifiers, "");
    lv_obj_set_pos(refs_modifiers, 108, ROW_BAT + 2);

    widget_nice_oled_output_init();
    widget_nice_oled_battery_init();
    widget_nice_oled_layer_init();
    widget_nice_oled_mods_init();
    widget_nice_oled_wpm_init();

    return screen;
}
