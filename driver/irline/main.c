/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include "ir.h"
#include "barcode_reader.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY          (tskIDLE_PRIORITY + 1UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

/*!
 * @brief Launches the tasks that needs to run
 * @param[in] void -
 * @return -
 */
void
vLaunch(void)
{
    sleep_ms(3000);

    TaskHandle_t detect_lines_task;
    if (xTaskCreate(detectLines,
                    "DetectLinesTask",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    8,
                    &detect_lines_task)
        != pdPASS)
    {
        printf("Failed to create DetectLinesTask\n");
        return;
    }
    else
    {
        printf("Successfully created DetectLinesTask\n");
    }

    // TaskHandle_t barcode_launch_task;
    // if (xTaskCreate(barcodeLaunch,
    //                 "BarcodeLaunchTask",
    //                 configMINIMAL_STACK_SIZE,
    //                 NULL,
    //                 8,
    //                 &barcode_launch_task)
    //     != pdPASS)
    // {
    //     printf("Failed to create BarcodeLaunchTask\n");
    //     return;
    // }
    // else
    // {
    //     printf("Successfully created BarcodeLaunchTask\n");
    // }
    barcodeLaunch();

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init
    // is called) (note we only do this in NO_SYS mode, because
    // cyw43_arch_freertos takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif
    // sleep_ms(3000);
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int
main(void)
{
    stdio_init_all();

    vLaunch();

    return 0;
}
