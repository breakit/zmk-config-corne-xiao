/*
 * Custom ZMK status screen showing the battery level of BOTH split halves on
 * the central (left) display, on a single line: "L<L%> R<R%>".
 *
 * Subscribes to:
 *   - zmk_battery_state_changed            (local / left / central battery)
 *   - zmk_peripheral_battery_state_changed (peripheral / right battery)
 *
 * Requires CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y and
 * CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(two_battery, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>

struct battery_state {
    uint8_t level;
    bool connected;
};

static lv_obj_t *local_label;
static lv_obj_t *periph_label;

static void local_batt_update_cb(struct battery_state state) {
    char text[10] = {};
    if (state.connected) {
        snprintf(text, sizeof(text), "L%3u%%", state.level);
    } else {
        snprintf(text, sizeof(text), "L---");
    }
    if (local_label) {
        lv_label_set_text(local_label, text);
    }
}

static struct battery_state local_batt_get_state(const zmk_event_t *_eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(_eh);
    return (struct battery_state){
        .level = ev ? ev->state_of_charge : 0,
        .connected = (ev != NULL),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(local_batt_widget, struct battery_state, local_batt_update_cb,
                            local_batt_get_state)
ZMK_SUBSCRIPTION(local_batt_widget, zmk_battery_state_changed);

static void periph_batt_update_cb(struct battery_state state) {
    char text[10] = {};
    if (state.connected) {
        snprintf(text, sizeof(text), "R%3u%%", state.level);
    } else {
        snprintf(text, sizeof(text), "R---");
    }
    if (periph_label) {
        lv_label_set_text(periph_label, text);
    }
}

static struct battery_state periph_batt_get_state(const zmk_event_t *_eh) {
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(_eh);
    return (struct battery_state){
        .level = ev ? ev->state_of_charge : 0,
        .connected = (ev != NULL),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(periph_batt_widget, struct battery_state, periph_batt_update_cb,
                            periph_batt_get_state)
ZMK_SUBSCRIPTION(periph_batt_widget, zmk_peripheral_battery_state_changed);

/* Strong override of the weak default status screen in zmk_display_status_screen(). */
lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    // Local (this half) battery, shown by both the central and peripheral halves.
    local_label = lv_label_create(screen);
    lv_obj_align(local_label, LV_ALIGN_LEFT_MID, 4, 0);
    local_batt_widget_init();

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    // Peripheral (other half) battery: only the central half has access to it.
    periph_label = lv_label_create(screen);
    lv_obj_align(periph_label, LV_ALIGN_CENTER, 8, 0);
    periph_batt_widget_init();
#endif

    return screen;
}
