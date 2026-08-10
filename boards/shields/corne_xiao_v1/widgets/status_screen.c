/*
 * Custom ZMK status screen for the Corne Xiao v1 (LVGL 9 / ZMK main).
 *
 * Central (left) display, 128x32. Shows, on a single screen:
 *   - both halves' battery: "L<l>% R<r>%" (peripheral fetched over BLE)
 *   - WPM readout + a playful Bongo-Cat style cat that "plays" (bounces)
 *     whenever typing is detected
 *   - the active layer name
 *
 * Requires CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y and
 * CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nice_status, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/wpm.h>

struct status_state {
    uint8_t local_batt;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    uint8_t wpm;
    const char *layer;
#endif
};

struct periph_batt_state {
    uint8_t level;
};

static lv_obj_t *batt_label;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static lv_obj_t *layer_label;
static lv_obj_t *wpm_label;
static lv_obj_t *cat_label;
#endif

/* Latest peripheral (right half) battery level, fed by its own event. */
static uint8_t periph_batt_level;
static bool periph_batt_known;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
/* Bongo-Cat style cat: bobs up and down while typing. */
static bool cat_playing;

static void cat_anim_cb(void *var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, v); }

static int32_t cat_anim_path(const lv_anim_t *a) { return lv_anim_path_bounce(a); }

static void start_cat_anim(lv_obj_t *cat) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, cat);
    lv_anim_set_exec_cb(&a, cat_anim_cb);
    lv_anim_set_values(&a, 0, -4);
    lv_anim_set_duration(&a, 140);
    lv_anim_set_repeat_count(&a, 2);
    lv_anim_set_path_cb(&a, cat_anim_path);
    lv_anim_start(&a);
}
#endif /* CONFIG_ZMK_SPLIT_ROLE_CENTRAL */

static void render(struct status_state s) {
    char text[24] = {};

    if (batt_label) {
        if (periph_batt_known) {
            snprintf(text, sizeof(text), "L%u%% R%u%%", s.local_batt, periph_batt_level);
        } else {
            snprintf(text, sizeof(text), "L%u%%", s.local_batt);
        }
        lv_label_set_text(batt_label, text);
    }

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (layer_label) {
        lv_label_set_text_fmt(layer_label, "%s", s.layer ? s.layer : "BASE");
    }

    if (wpm_label) {
        snprintf(text, sizeof(text), "%u", s.wpm);
        lv_label_set_text(wpm_label, text);
    }

    if (cat_label) {
        lv_label_set_text(cat_label, s.wpm > 0 ? "^='-'" : " _ /   \\_");
        if (s.wpm > 0 && !cat_playing) {
            start_cat_anim(cat_label);
            cat_playing = true;
        } else if (s.wpm == 0 && cat_playing) {
            cat_playing = false;
        }
    }
#endif
}

static void status_update_cb(struct status_state s) { render(s); }

static struct status_state status_get_state(const zmk_event_t *eh) {
    struct status_state s = {0};
    s.local_batt = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    s.wpm = zmk_wpm_get_state();
    s.layer = NULL;

    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    if (index != 0) {
        s.layer = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index));
    }
#endif
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(status_widget, struct status_state, status_update_cb,
                            status_get_state)
ZMK_SUBSCRIPTION(status_widget, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(status_widget, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(status_widget, zmk_wpm_state_changed);
#endif

/* Dedicated listener that captures the peripheral (right half) battery. */
static void periph_batt_update_cb(struct periph_batt_state s) {
    periph_batt_level = s.level;
    periph_batt_known = true;
}

static struct periph_batt_state periph_batt_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);
    return (struct periph_batt_state){.level = ev->state_of_charge};
}

ZMK_DISPLAY_WIDGET_LISTENER(periph_status, struct periph_batt_state, periph_batt_update_cb,
                            periph_batt_get_state)
ZMK_SUBSCRIPTION(periph_status, zmk_peripheral_battery_state_changed);

/* Strong override of the weak default status screen in display/main.c. */
lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    cat_label = lv_label_create(screen);
    lv_obj_align(cat_label, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_label_set_text(cat_label, " \\ /  _");

    wpm_label = lv_label_create(screen);
    lv_obj_align(wpm_label, LV_ALIGN_TOP_RIGHT, -14, 0);
    lv_label_set_text(wpm_label, "0");

    layer_label = lv_label_create(screen);
    lv_obj_align(layer_label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_label_set_text(layer_label, "BASE");
#endif

    batt_label = lv_label_create(screen);
    lv_obj_align(batt_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);
    lv_label_set_text(batt_label, "L--%");

    status_widget_init();
    periph_status_init();
    return screen;
}
