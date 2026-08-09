# Compile the custom two-battery status screen widget into the ZMK app.
target_sources(app PRIVATE
    boards/shields/corne_xiao_v1/widgets/two_battery_status.c
)
