/*
 * Stock-look custom OLED status screen for the Corne Xiao v1 central (left)
 * half.
 *
 * Reproduces the ZMK built-in status screen's battery rendering (charge glyph
 * + battery level symbol + percentage), with two labels side-by-side at the
 * top-right: the left one is the central (left) half and the one to its right
 * is the peripheral (right) half.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>

#include <zmk/battery.h>
#include <zmk/usb.h>
#include <zmk/split/central.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>

#define BAT_Y 2
#define BAT_R_X 0        /* central (L) battery, top-right like the built-in */
#define BAT_R2_X (-44)   /* peripheral (R) battery, side-by-side to its left */

static lv_obj_t *refs_battery_label;   /* central (L) battery */
static lv_obj_t *refs_r_battery_label; /* peripheral (R) battery */

struct status_state {
    uint8_t battery;
    uint8_t peripheral_battery;
    bool charging;
};

static struct status_state status_state_get(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *batt_ev = as_zmk_battery_state_changed(eh);

    return (struct status_state){
        .battery = (batt_ev != NULL) ? batt_ev->state_of_charge : zmk_battery_state_of_charge(),
        .peripheral_battery = 0,
        .charging = zmk_usb_is_powered(),
    };
}

/* Render a label exactly like the built-in battery_status widget:
 * [CHG ]BATTERY_x %  (charge glyph when charging, then level bar, then %).
 */
static void render_battery(lv_obj_t *label, uint8_t level, bool charging) {
    char text[16] = {};

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    if (charging) {
        strcpy(text, LV_SYMBOL_CHARGE " ");
    }
#endif

    if (level > 95) {
        strcat(text, LV_SYMBOL_BATTERY_FULL);
    } else if (level > 65) {
        strcat(text, LV_SYMBOL_BATTERY_3);
    } else if (level > 35) {
        strcat(text, LV_SYMBOL_BATTERY_2);
    } else if (level > 5) {
        strcat(text, LV_SYMBOL_BATTERY_1);
    } else {
        strcat(text, LV_SYMBOL_BATTERY_EMPTY);
    }

    char perc[5] = {};
    snprintf(perc, sizeof(perc), " %2u%%", level);
    strcat(text, perc);

    lv_label_set_text(label, text);
}

static void redraw_cb(struct status_state *s) {
    render_battery(refs_battery_label, s->battery, s->charging);

    bool show_periph = s->peripheral_battery > 0 && s->peripheral_battery <= 100;
    if (show_periph && IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)) {
        render_battery(refs_r_battery_label, s->peripheral_battery, false);
    } else {
        lv_label_set_text(refs_r_battery_label, "-");
    }
}

static void battery_update_cb(struct status_state s) { redraw_cb(&s); }

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

ZMK_DISPLAY_WIDGET_LISTENER(widget_nice_oled_battery, struct status_state, battery_update_cb,
                            status_state_get)

ZMK_SUBSCRIPTION(widget_nice_oled_battery, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
ZMK_SUBSCRIPTION(widget_nice_oled_periph_batt, zmk_peripheral_battery_state_changed);
#endif
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_nice_oled_battery, zmk_usb_conn_state_changed);
#endif

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);
    lv_obj_set_style_border_width(screen, 0, 0);

    /* Top-right, mirroring the built-in battery position. */
    refs_battery_label = lv_label_create(screen);
    render_battery(refs_battery_label, 0, false);
    lv_obj_align(refs_battery_label, LV_ALIGN_TOP_RIGHT, BAT_R_X, BAT_Y);

    /* Peripheral (right) battery, side-by-side to the left of the central one. */
    refs_r_battery_label = lv_label_create(screen);
    lv_label_set_text(refs_r_battery_label, "-");
    lv_obj_align(refs_r_battery_label, LV_ALIGN_TOP_RIGHT, BAT_R2_X, BAT_Y);

    widget_nice_oled_battery_init();
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    widget_nice_oled_periph_batt_init();
#endif

    return screen;
}
