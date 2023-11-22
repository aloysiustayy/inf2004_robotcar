/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"

#include "ssi.h"
#include "cgi.h"
#include "ir.h"
#include "barcode_reader.h"
#include "motor.h"
#include "encoder.h"
#include "magnetometer.h"
#include "obstacle.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

#define WIFI_SSID "hargaowithchili"
#define WIFI_PASSWORD "athmqwer"
#define IP_ADDRESS "172.20.10.10"
// #define IP_ADDRESS "192.168.1.20"
#define NETMASK "255.255.255.0"
#define GATEWAY "172.20.10.1"
// #define GATEWAY "192.168.1.254"

QueueHandle_t ir_queue_handle;
QueueHandle_t barcode_queue_handle;
QueueHandle_t ultrasonic_queue_handle;

/*!
 * @brief Receives char[100] from Queue to print on serial monitor
 * @param[in] params Any parameter that could be passed in
 * @return -
 */
void task_recv_ir_message(__unused void *params)
{
    char receivedValue[100];

    for (;;)
    {
        vTaskDelay(100);
        if (ir_queue_handle == NULL)
        {
            ir_queue_handle = IRMessageHandler();
        }

        if (xQueueReceive(ir_queue_handle, &receivedValue, portMAX_DELAY) == pdPASS)
        {
            printf("Motor: %s\n", receivedValue);

            // Set for test var in ssi.h
            // snprintf(web_data, sizeof(web_data), "%s", receivedValue);

            // Set web_data in ssi.h
            set_ir_motor_command(receivedValue);

            // Set motor_command in motor.c
            set_motor_command(receivedValue);
        }
    }
}

void task_recv_barcode_message(__unused void *params)
{
    char receivedValue[100];

    int counter = 0;
    for (;;)
    {
        vTaskDelay(100);
        if (barcode_queue_handle == NULL)
        {
            barcode_queue_handle = BarcodeMessageHandler();
        }

        if (xQueueReceive(barcode_queue_handle, &receivedValue, portMAX_DELAY) == pdPASS)
        {
            printf("Barcode: %s\n", receivedValue);
            set_barcode_data(receivedValue);
        }
    }
}

/*!
 * @brief Receives char[100] from respective Queue
 * @param[in] params Any parameter that could be passed in
 * @return -
 */
void task_recv_ultrasonic_message(__unused void *params)
{
    char receivedValue[100];

    for (;;)
    {
        vTaskDelay(100);
        if (ultrasonic_queue_handle == NULL)
        {
            ultrasonic_queue_handle = UltrasonicMessageHandler();
        }

        if (xQueueReceive(ultrasonic_queue_handle, &receivedValue, portMAX_DELAY) == pdPASS)
        {
            printf("Motor: %s\n", receivedValue);

            // Set motor_command in motor.c
            set_motor_command(receivedValue);

            // Send to webpage?

        }
    }
}
void start_recv_msg_task()
{
    TaskHandle_t recv_ir_task;
    xTaskCreate(task_recv_ir_message,
                "task_recv_ir_messageTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &recv_ir_task);

    TaskHandle_t recv_barcode_task;
    xTaskCreate(task_recv_barcode_message,
                "task_recv_barcode_messageTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &recv_barcode_task);

    TaskHandle_t recv_ultrasonic_task;
    xTaskCreate(task_recv_ultrasonic_message,
                "task_recv_ultrasonic_messageTask",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &recv_ultrasonic_task);
}

void main_task(__unused void *params)
{

    ip_addr_t ip;
    ip_addr_t netmask;
    ip_addr_t gateway;

    ip4addr_aton(IP_ADDRESS, &ip);
    ip4addr_aton(NETMASK, &netmask);
    ip4addr_aton(GATEWAY, &gateway);
    cyw43_arch_init();

    //  RUNNING PICO-W AS AN ACCESS POINT
    // cyw43_arch_enable_ap_mode("ConnectMe!", "password123",
    // CYW43_AUTH_WPA2_AES_PSK); cyw43_arch_lwip_begin(); struct netif *n =
    // &cyw43_state.netif[CYW43_ITF_AP]; netif_set_addr(n, &ip, &netmask,
    // &gateway); netif_set_up(n); cyw43_arch_lwip_end();

    cyw43_arch_enable_sta_mode();
    cyw43_arch_lwip_begin();
    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
    netif_set_hostname(n, "Team77Pico");
    dhcp_stop(n);
    netif_set_addr(n, &ip, &netmask, &gateway);
    netif_set_up(n);
    cyw43_arch_lwip_end();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(
               WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
    {
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");

    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");
    printf("Starting server at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    // Infinite loop
    while (true)
    {
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void vLaunch(void)
{
    TaskHandle_t task;
    xTaskCreate(main_task,
                "webserverThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 1UL,
                &task);

    TaskHandle_t barcodetask;
    xTaskCreate(barcodeLaunch,
                "barcodeThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &barcodetask);

    

    TaskHandle_t detectLinesTask;
    xTaskCreate(detectLines,
                "detectLinesThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &detectLinesTask);


    TaskHandle_t encoderTask;
    xTaskCreate(encoder_main,
                "encoderThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &encoderTask);

    TaskHandle_t ultrasonicTask;
    xTaskCreate(ultrasonic_main,
                "ultrasonicThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                &ultrasonicTask);

    motor_main();
    
    // magnetometer_main(); // Run mag

    // Start all queue tasks
    start_recv_msg_task();

    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    adc_init();
    // init_i2c();

    vLaunch();
    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
    rtos_name = "FreeRTOS";

    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
    return 0;
}