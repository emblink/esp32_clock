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
#include "driver/gpio.h"
#include "hal/gpio_types.h"

#include "spi.h"
#include "max7219.h"
#include "font.h"
#include "wifi.h"
#include "sntp.h"

// Define event types
typedef enum {
    WIFI_CONNECTED_EVENT,
    WIFI_DISCONNECTED_EVENT,
    TIME_UPDATE_EVENT,
    UPDATE_STATUS_LED,
} EventType;

#define STATUS_LED_PIN GPIO_NUM_8
#define TAG "app_main"

const TickType_t DelayBetweenUpdates = pdMS_TO_TICKS(1000);
static QueueHandle_t eventQueue = NULL;
TimerHandle_t updateTimer = NULL;
TimerHandle_t statusLedTimer = NULL;

static void processClockMode(void)
{
    Time t = getTime();
    ESP_LOGI(TAG, "Time: %d:%d:%d", t.hour, t.min, t.sec);
    bool blink = (t.sec & 0x01);
   
    max7219SendSymbol(MAX7219_NUMBER_0, fontGetNumberArray(t.hour / 10));
    if (blink) {
        max7219SendSymbol(MAX7219_NUMBER_1, fontAddDots(fontGetNumberArray(t.hour % 10), FONT_SYMBOL_RIGHT_SHIFT));
        max7219SendSymbol(MAX7219_NUMBER_2, fontAddDots(fontGetNumberArrayShifted(t.min / 10, FONT_SYMBOL_RIGHT_SHIFT, 2), FONT_SYMBOL_LEFT_SHIFT));
    } else {
        max7219SendSymbol(MAX7219_NUMBER_1, fontGetNumberArray(t.hour % 10));
        max7219SendSymbol(MAX7219_NUMBER_2, fontGetNumberArrayShifted(t.min / 10, FONT_SYMBOL_RIGHT_SHIFT, 2));
    }
    max7219SendSymbol(MAX7219_NUMBER_3, fontGetNumberArrayShifted(t.min % 10, FONT_SYMBOL_RIGHT_SHIFT, 2));
}

void processStatusLed()
{
    static uint32_t state = 0;
    gpio_set_level(STATUS_LED_PIN, state);
    state ^= 1;
}

static void onWifiConnectionCallback(bool connected)
{
    EventType event = (connected) ? WIFI_CONNECTED_EVENT : WIFI_DISCONNECTED_EVENT;
    // Send the event to the queue
    if (eventQueue != NULL) {
        xQueueSend(eventQueue, &event, portMAX_DELAY);
    }
}

static void onTimeSync(void)
{
    // Send time sync event to the queue
    EventType event = TIME_UPDATE_EVENT;
    xTimerReset(updateTimer, 0);
    if (eventQueue != NULL) {
        xQueueSend(eventQueue, &event, portMAX_DELAY);
    }
}

static void updateTimerCallback(TimerHandle_t xTimer)
{
    (void) xTimer;
    EventType event = TIME_UPDATE_EVENT;
    if (eventQueue != NULL) {
        xQueueSend(eventQueue, &event, portMAX_DELAY);
    }
}

static void statusLedTimerCallback(TimerHandle_t xTimer)
{
    (void) xTimer;
    EventType event = UPDATE_STATUS_LED;
    if (eventQueue != NULL) {
        xQueueSend(eventQueue, &event, portMAX_DELAY);
    }
}

void app_main(void)
{
    // Create the event queue
    eventQueue = xQueueCreate(10, sizeof(EventType)); // 10 is the queue length
    if (eventQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return;
    }

    vSpiInit();
    max7219Init();
    wifi_init(onWifiConnectionCallback);
    sntpInit(onTimeSync);
    gpio_set_direction(STATUS_LED_PIN, GPIO_MODE_OUTPUT);

    updateTimer = xTimerCreate("TimeUpdateTimer", pdMS_TO_TICKS(1000), pdTRUE,
                     ( void * ) 0, updateTimerCallback);
    statusLedTimer = xTimerCreate("StatusLedUpdateTimer", pdMS_TO_TICKS(500), pdTRUE,
                     ( void * ) 0, statusLedTimerCallback);
    xTimerStart(updateTimer, 0);
    xTimerStart(statusLedTimer, 0);

    while (1)
    {
        EventType receivedEvent;
        if (xQueueReceive(eventQueue, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            switch (receivedEvent) {
                case WIFI_CONNECTED_EVENT:
                    obtain_time();
                    xTimerChangePeriod(statusLedTimer, pdMS_TO_TICKS(2000), 0);
                    ESP_LOGI(TAG, "Wi-Fi Connected");
                    break;

                case WIFI_DISCONNECTED_EVENT:
                    xTimerChangePeriod(statusLedTimer, pdMS_TO_TICKS(500), 0);
                    ESP_LOGI(TAG, "Wi-Fi Disconnected");
                    break;

                case TIME_UPDATE_EVENT:
                    processClockMode();
                    break;
                    
                case UPDATE_STATUS_LED:
                    processStatusLed();
                    break;

                default:
                    break;
            }
        }
    }
}
