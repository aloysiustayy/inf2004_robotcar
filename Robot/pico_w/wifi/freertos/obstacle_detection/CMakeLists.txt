add_executable(obstacle_detection obstacle.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(obstacle_detection pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(obstacle_detection 1)
pico_enable_stdio_uart(obstacle_detection 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(obstacle_detection)

# add url via pico_set_program_url
example_auto_set_url(obstacle_detection)