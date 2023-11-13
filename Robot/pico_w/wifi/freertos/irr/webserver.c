#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwipopts.h"
#include "lwip/apps/httpd.h"

#include "ssi.h"
#include "cgi.h"

// WIFI Credentials - take care if pushing to github!
// const char WIFI_SSID[]     = "Aaron";
// const char WIFI_PASSWORD[] = "aaronhotpot2000";

void
webserver(__unused void *params)
{
    // stdio_init_all();

    printf("fuck u? 222");
    cyw43_arch_enable_sta_mode();

    printf("fuck u? 333");
    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(
               "hargaowithchili", "athmqwer", CYW43_AUTH_WPA2_AES_PSK, 30000)
           != 0)
    {
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");
    printf("Starting server at %s yay\n",
           ip4addr_ntoa(netif_ip4_addr(netif_list)));

    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");

    printf("Starting server at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    int counter = 1;
    // Infinite loop
    while (1)
    {
        vTaskDelay(100);
        printf("fucky %d\n", counter);
        counter++;
    }
}

void
webtask()
{
    printf("fuck u? 000  ");
    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed.");
    }
    printf("fuck u? 111  ");

    TaskHandle_t webserver_task;
    if (xTaskCreate(
            webserver, "webserver_task", 1024, NULL, 10, &webserver_task)
        != pdPASS)
    {
        printf("Failed to create webserver_task\n");
        return;
    }
    else
    {
        printf("Successfully created webserver_task\n");
    }
}