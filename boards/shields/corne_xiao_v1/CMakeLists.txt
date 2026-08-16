# SPDX-License-Identifier: MIT
# Compile the custom OLED status screen only when CUSTOM is selected and this
# half is the central role (left). The peripheral (right) half stays on the
# built-in screen.
if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE
      zmk_nice_oled/nice_oled_status.c)
  endif()
endif()
