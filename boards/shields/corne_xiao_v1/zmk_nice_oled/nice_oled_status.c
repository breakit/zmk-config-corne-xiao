/*
 * SINGLE-LABEL PROOF for the memory/stack experiment.
 * Minimal custom status screen that creates exactly ONE label. This is the
 * smallest known-to-hang repro under the default 2048-byte LVGL heap. If it
 * boots after raising LV_Z_MEM_POOL_SIZE, the memory hypothesis is confirmed.
 */

#include <lvgl.h>
#include <zmk/display/status_screen.h>

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_t *lbl = lv_label_create(screen);
    lv_label_set_text(lbl, "MEM?");
    lv_obj_set_pos(lbl, 4, 4);

    return screen;
}
