#include "sdkconfig.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "spi.h"
#include "max7219.h"
#include "font.h"

const TickType_t DelayBetweenUpdates = pdMS_TO_TICKS(1000);

void app_main(void) {
    vSpiInit();
    max7219Init();

    while (1)
    {
        max7219SendSymbol(MAX7219_NUMBER_0, fontGetNumberArray(1));
        max7219SendSymbol(MAX7219_NUMBER_1, fontGetSpaceArray());
        max7219SendSymbol(MAX7219_NUMBER_2, fontGetSpaceArray());
        max7219SendSymbol(MAX7219_NUMBER_3, fontGetSpaceArray());
        vTaskDelay(DelayBetweenUpdates);
        max7219SendSymbol(MAX7219_NUMBER_1, fontGetNumberArray(2));
        vTaskDelay(DelayBetweenUpdates);
        max7219SendSymbol(MAX7219_NUMBER_2, fontGetNumberArray(3));
        vTaskDelay(DelayBetweenUpdates);
        max7219SendSymbol(MAX7219_NUMBER_3, fontGetNumberArray(4));
        vTaskDelay(DelayBetweenUpdates);
        max7219SendSymbol(MAX7219_NUMBER_0, fontGetSpaceArray());
        max7219SendSymbol(MAX7219_NUMBER_1, fontGetSpaceArray());
        max7219SendSymbol(MAX7219_NUMBER_2, fontGetSpaceArray());
        max7219SendSymbol(MAX7219_NUMBER_3, fontGetSpaceArray());
        vTaskDelay(DelayBetweenUpdates);
    }
}
