
    include(pico_sdk_import.cmake)

    message("Running makefsdata python script")
    execute_process(COMMAND
        py convert_html.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

    add_executable(irr_sensor_sys
        main.c
        ir.c
        ir.h    
        barcode_reader.c    
        barcode_reader.h   
        motor.c
        motor.h
        encoder.c
        encoder.h 
        magnetometer.c
        magnetometer.h
        ssi.h
        cgi.h
        obstacle.c
        obstacle.h
        global_defined.h
        gpio_handler.c
        gpio_handler.h
        )
    
    # target_compile_definitions(irr_sensor_sys PRIVATE
    #     # WIFI_SSID=\"${WIFI_SSID}\"
    #     # WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    #     NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
    #     LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
    #     )
    target_include_directories(irr_sensor_sys PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
    
    )
target_link_libraries(irr_sensor_sys
    # pico_cyw43_arch_lwip_sys_freertos
    pico_cyw43_arch_lwip_threadsafe_background
    # pico_cyw43_arch_none
    pico_lwip_http
    pico_stdlib
    pico_lwip_iperf
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    hardware_pwm
    hardware_adc
    hardware_i2c
    hardware_spi
    hardware_uart
    pico_ultrasonic
    )
    
pico_enable_stdio_usb(irr_sensor_sys 1)
pico_enable_stdio_uart(irr_sensor_sys 0)

pico_add_extra_outputs(irr_sensor_sys)
# add url via pico_set_program_url
example_auto_set_url(irr_sensor_sys)
