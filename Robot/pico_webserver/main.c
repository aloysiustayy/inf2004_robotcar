#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"

// WIFI Credentials - take care if pushing to github!
// const char WIFI_SSID[]     = "Max";
// const char WIFI_PASSWORD[] = "12345678";

int main()
{
    stdio_init_all();

    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(
               "hargaowithchili", "athmqwer", CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
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

    // sleep_ms(5000);
    // Infinite loop
    while (1)
        ;
}