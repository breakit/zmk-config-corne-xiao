# SPDX-License-Identifier: MIT
# Experiment: compile the single-label custom screen only when CUSTOM is
# selected and this half is the central role (left).
if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE
      zmk_nice_oled/nice_oled_status.c)
  endif()
endif()
