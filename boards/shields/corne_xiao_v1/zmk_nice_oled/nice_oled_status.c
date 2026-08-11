/*
 * Minimal widget-set test for the memory experiment.
 * Battery + WPM number labels driven by the ZMK_DISPLAY_WIDGET_LISTENER
 * pattern (the same code that hung at the 2048-byte heap default). Confirms
 * the raised LVGL heap also covers the listener/state-read render path.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>

#include <zmk/battery.h>
#include <zmk/wpm.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/wpm_state_changed.h>

static lv_obj_t *batt_label;
static lv_obj_t *wpm_label;

struct min_state {
    uint8_t battery;
    uint8_t wpm;
};

static void set_battery(struct min_state s) {
    char text[12] = {};
    snprintf(text, sizeof(text), "%d%%", s.battery);
    lv_label_set_text(batt_label, text);
}

static struct min_state batt_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    return (struct min_state){.battery = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
                              .wpm = 0};
}

static void set_wpm(struct min_state s) {
    char text[12] = {};
    snprintf(text, sizeof(text), "w:%d", s.wpm);
    lv_label_set_text(wpm_label, text);
}

static struct min_state wpm_get_state(const zmk_event_t *eh) {
    return (struct min_state){.battery = 0, .wpm = zmk_wpm_get_state()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_min_batt, struct min_state, set_battery, batt_get_state)
ZMK_DISPLAY_WIDGET_LISTENER(widget_min_wpm, struct min_state, set_wpm, wpm_get_state)

ZMK_SUBSCRIPTION(widget_min_batt, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(widget_min_wpm, zmk_wpm_state_changed);

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    batt_label = lv_label_create(screen);
    lv_label_set_text(batt_label, "BATT");
    lv_obj_set_pos(batt_label, 4, 4);

    wpm_label = lv_label_create(screen);
    lv_label_set_text(wpm_label, "WPM");
    lv_obj_set_pos(wpm_label, 4, 18);

    widget_min_batt_init();
    widget_min_wpm_init();

    return screen;
}
