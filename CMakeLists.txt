# Compile the custom status-screen widget into the ZMK app.
# Only needed when the display is enabled (no display on the corne_xiao_v2).
if(CONFIG_ZMK_DISPLAY)
  target_sources(app PRIVATE
      boards/shields/corne_xiao_v1/widgets/status_screen.c
  )
endif()
