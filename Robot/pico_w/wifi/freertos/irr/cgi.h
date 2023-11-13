#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"

char maze_data[1024];

void set_maze_data(char *data)
{
    snprintf(maze_data, sizeof(maze_data), "%s", data);
}

// CGI handler which is run when a request for /led.cgi is detected
const char *
cgi_led_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if an request for LED has been made (/led.cgi?led=x)
    // if (strcmp(pcParam[0], "led") == 0)
    // {
    //     // Look at the argument to check if LED is to be turned on (x=1) or
    //     // off (x=0)
    //     if (strcmp(pcValue[0], "0") == 0)
    //     {
    //         printf("turning off led");
    //         // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    //         printf("led off");
    //     }
    //     else if (strcmp(pcValue[0], "1") == 0)
    //     {
    //         printf("turning on led");
    //         // TODO: found the error guy. this guy made the whole ass hanged
    //         // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //         printf("led on");
    //     }
    // }

    // Send the maze_data back to the client-side
    httpd_resp_send(iIndex, maze_data, strlen(maze_data));

    return NULL;
    // Send the index page back to the user
    // return "/index.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    {// Html request for "/led.cgi" triggers cgi_handler
     "/led.cgi",
     cgi_led_handler},
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 1);
}