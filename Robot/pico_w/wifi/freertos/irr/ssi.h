#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "hardware/adc.h"

// SSI tags - tag length limited to 8 bytes by default
const char *ssi_tags[] = {"volt", "temp", "led", "message"};

char ir_motor_data[100];
char barcode_data[100];

void set_ir_motor_command(char *data)
{
    snprintf(ir_motor_data, sizeof(ir_motor_data), "%s", data);
}

void set_barcode_data(char *data)
{
    snprintf(barcode_data, sizeof(barcode_data), "%s", data);
}

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{
    // adc_select_input(4);
    size_t printed;

    switch (iIndex)
    {
    case 0: // volt
    {
        // printf("what 0");
        // const float voltage = adc_read() * 3.3f / (1 << 12);
        // printed             = snprintf(pcInsert, iInsertLen, "%f",
        // voltage);
        printed = snprintf(pcInsert, iInsertLen, "%s", barcode_data);
    }
    break;
    case 1: // temp
    {
        // printf("what 1");

        // const float voltage = adc_read() * 3.3f / (1 << 12);
        // const float tempC   = 27.0f - (voltage - 0.706f) / 0.001721f;
        printed = snprintf(pcInsert, iInsertLen, "%s", ir_motor_data);
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

        printed = snprintf(pcInsert, iInsertLen, "I am piccoooo!");
    }
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
    // Initialise ADC (internal pin)
    // adc_init();
    // adc_set_temp_sensor_enabled(true);
    // adc_select_input(4);

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}
