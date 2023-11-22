#include <stdio.h>
#include <unistd.h>
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"

// CGI handler which is run when a request for /led.cgi is detected
const char *cgi_led_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if an request for LED has been made (/led.cgi?led=x)
    if (strcmp(pcParam[0], "led") == 0)
    {
        // Look at the argument to check if LED is to be turned on (x=1) or off (x=0)
        if (strcmp(pcValue[0], "0") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        else if (strcmp(pcValue[0], "1") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }

    // Send the index page back to the user
    return "/index.shtml";
}

// CGI handler which is run when a request for /maze.cgi is detected
const char *
cgi_maze_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // // /maze.cgi this is calling.
    // // return "/index.shtml";
    // printf("Maze data is %s\n", get_maze_data());
    // // char *maze = malloc(sizeof(char) * 350);
    // // Open a file for writing
    // FILE *file = fopen("./test.shtml", "w");

    // // Check if the file is successfully opened
    // if (file == NULL)
    // {
    //     perror("Error opening file!!!");
    // }

    // // Write the char* data to the file
    // if (fprintf(file, "%s\n", get_maze_data()) < 0)
    // {
    //     perror("Error writing to file!!!");
    //     fclose(file); // Close the file before exiting
    // }

    // // Close the file
    // fclose(file);
    // // return get_maze_data();
    // return "/test.shtml";
    return NULL;
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    {"/led.cgi", cgi_led_handler},
    {"/maze.cgi",
     cgi_maze_handler},
};

void cgi_init(void)
{
    printf("LWIP_ARRAYSIZE: %d\n", LWIP_ARRAYSIZE(cgi_handlers));
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}