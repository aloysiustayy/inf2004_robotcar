#ifndef SSI_H
#define SSI_H
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"

// SSI tags - tag length limited to 8 bytes by default
const char *ssi_tags[] = {"volt", "temp", "led", "message", "maze1", "maze2"};

char ir_motor_data[100];
char barcode_data[100];
char *maze_data;

void set_ir_motor_command(char *data)
{
    snprintf(ir_motor_data, sizeof(ir_motor_data), "%s", data);
}

void set_barcode_data(char *data)
{
    snprintf(barcode_data, sizeof(barcode_data), "%s", data);
}

void set_maze_data(char *data)
{
    printf("\nData is %s\n", data);
    maze_data = malloc(sizeof(char) * (strlen(data) + 1));

    snprintf(maze_data, strlen(data) + 1, "%s", data);
    printf("\nMaze is %s\n", maze_data);

    printf("Data size: %d\nMaze size: %d\n", strlen(data), strlen(maze_data));
}
char *get_maze_data()
{
    char *return_data = malloc(strlen(maze_data));
    strcpy(return_data, maze_data);
    return return_data;
}

// Get maze text based on
char *get_maze_text(int index)
{
    int mid = strlen(get_maze_data()) / 2;

    char *first_half = (char *)malloc(mid + 1);                              // Allocate memory for the first half
    char *second_half = (char *)malloc((strlen(get_maze_data()) - mid) + 1); // Allocate memory for the second half

    strncpy(first_half, get_maze_data(), mid);                                  // Copy the first half of the array to first_half
    strncpy(second_half, get_maze_data() + mid, strlen(get_maze_data()) - mid); // Copy the second half of the array to second_half

    first_half[mid] = '\0';                            // Add a null terminator to the end of the first half
    second_half[strlen(get_maze_data()) - mid] = '\0'; // Add a null terminator to the end of the second half

    if (index == 4)
        return first_half;
    else if (index == 5)
        return second_half;
    else
        return "";
}

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{
    // adc_select_input(4);
    size_t printed;
    // char arr[] = "Hello, World!";

    // printf("First half: %s\n", first_half);
    // printf("Second half: %s\n", second_half);

    switch (iIndex)
    {
    case 0: // volt
    {
        // printf("what 0");
        // const float voltage = adc_read() * 3.3f / (1 << 12);
        // printed             = snprintf(pcInsert, iInsertLen, "%f",
        // voltage);
        printed = snprintf(pcInsert, iInsertLen, "%s", barcode_data);
        // printf("0 Insert Len at %d\n", iInsertLen);
    }
    break;
    case 1: // temp
    {
        // printf("what 1");

        // const float voltage = adc_read() * 3.3f / (1 << 12);
        // const float tempC   = 27.0f - (voltage - 0.706f) / 0.001721f;
        printed = snprintf(pcInsert, iInsertLen, "%s", ir_motor_data);
        // printf("1 Insert Len at %d\n", iInsertLen);
    }
    break;
    case 2: // led
    {
        // printf("what 2");

        // bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
        // if (led_status == true)
        // {
        //     printed = snprintf(pcInsert, iInsertLen, "ON");
        // }
        // else
        // {
        //     printed = snprintf(pcInsert, iInsertLen, "OFF");
        // }
    }
    break;
    case 3: // message
    {
        // printf("what 3");
        // printf("3 Insert Len at %d\n", iInsertLen);
        printed = snprintf(pcInsert, iInsertLen, "I am piccoooo!");
        // printed = snprintf(pcInsert, iInsertLen, first_half);
    }
    break;
    case 4: // maze1

        printed = snprintf(pcInsert, iInsertLen, get_maze_text(iIndex));
        break;
    case 5: // maze2

        printed = snprintf(pcInsert, iInsertLen, get_maze_text(iIndex));
        break;
    default:
        printed = 0;
        break;
    }
    // printed = snprintf(pcInsert, iInsertLen, "Test");
    // printf("printed is %s\n", printed);
    return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init()
{
    // Assuming a 10x10 grid
    int rows = 10;
    int cols = 10;

    // Initialize an array to store the values
    int grid[10][10];

    // Populate the grid with 0 (you can replace this with your actual values)
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            grid[i][j] = 0;
        }
    }

    // Initialize the string
    char md[2048]; // Adjust the size as needed

    // Use sprintf to format the string
    sprintf(md, "[");

    for (int i = 0; i < rows; i++)
    {
        sprintf(md + strlen(md), "[");
        for (int j = 0; j < cols; j++)
        {
            sprintf(md + strlen(md), "%d", grid[i][j]);

            // Add a comma if it's not the last element in the row
            if (j < cols - 1)
            {
                sprintf(md + strlen(md), ", ");
            }
        }
        sprintf(md + strlen(md), "]");

        // Add a comma if it's not the last row
        if (i < rows - 1)
        {
            sprintf(md + strlen(md), ", ");
        }
    }
    sprintf(md + strlen(md), "]");
    char *ms = "{ \"maze\":";
    char *me = "}";

    // Allocate memory for the concatenated string
    char *concatenated = (char *)malloc(strlen(ms) + strlen(md) + strlen(me) + 1);

    // Check if memory allocation was successful
    if (concatenated == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return; // Exit the function
    }

    // Construct the concatenated string
    strcpy(concatenated, ms);
    strcat(concatenated, md);
    strcat(concatenated, me);

    // Print the concatenated string for verification
    printf("Concatenated: %s\n", concatenated);

    // Set the maze data
    set_maze_data(concatenated);

    // Free the allocated memory
    free(concatenated);

    // Set up the SSI handler
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

#endif