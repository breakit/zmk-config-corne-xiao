# Compile the custom two-battery status screen widget into the ZMK app.
# Only needed when the display is enabled (no display on the corne_xiao_v2).
if(CONFIG_ZMK_DISPLAY)
  target_sources(app PRIVATE
      boards/shields/corne_xiao_v1/widgets/two_battery_status.c
  )
endif()
