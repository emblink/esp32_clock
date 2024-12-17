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
#include "wifi_manager.h"

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
static bool timeSynced = false;
static bool wifiConnected = false;

static void processClockMode(void)
{
    if (timeSynced) {
        Time t = getTime();
        // ESP_LOGI(TAG, "Time: %d:%d:%d", t.hour, t.min, t.sec);
        bool blink = (t.sec & 0x01);
    
        max7219SendSymbol(MAX7219_NUMBER_0, fontGetNumberArrayShifted(t.hour / 10, FONT_SYMBOL_RIGHT_SHIFT, 1));
        if (blink) {
            max7219SendSymbol(MAX7219_NUMBER_1, fontAddDots(fontGetNumberArray(t.hour % 10), FONT_SYMBOL_RIGHT_SHIFT));
            max7219SendSymbol(MAX7219_NUMBER_2, fontAddDots(fontGetNumberArrayShifted(t.min / 10, FONT_SYMBOL_RIGHT_SHIFT, 2), FONT_SYMBOL_LEFT_SHIFT));
        } else {
            max7219SendSymbol(MAX7219_NUMBER_1, fontGetNumberArray(t.hour % 10));
            max7219SendSymbol(MAX7219_NUMBER_2, fontGetNumberArrayShifted(t.min / 10, FONT_SYMBOL_RIGHT_SHIFT, 2));
        }
        max7219SendSymbol(MAX7219_NUMBER_3, fontGetNumberArrayShifted(t.min % 10, FONT_SYMBOL_RIGHT_SHIFT, 1));
    } else {
        if (wifiConnected) {
            static bool blink = true;
            if (blink) {
                max7219SendSymbol(MAX7219_NUMBER_0, fontGetCharArray('S'));
                max7219SendSymbol(MAX7219_NUMBER_1, fontGetCharArray('Y'));
                max7219SendSymbol(MAX7219_NUMBER_2, fontGetCharArray('N'));
                max7219SendSymbol(MAX7219_NUMBER_3, fontGetCharArray('C'));
            } else {
                max7219SendSymbol(MAX7219_NUMBER_0, fontGetSpaceArray());
                max7219SendSymbol(MAX7219_NUMBER_1, fontGetSpaceArray());
                max7219SendSymbol(MAX7219_NUMBER_2, fontGetSpaceArray());
                max7219SendSymbol(MAX7219_NUMBER_3, fontGetSpaceArray());
            }
            blink = !blink;
        } else {
            static int disconnectAnimFrame = 0;
            static const uint8_t dot[] = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
            if (MAX7219_NUMBER_COUNT <= disconnectAnimFrame) {
                for (int seg = MAX7219_NUMBER_0; seg < MAX7219_NUMBER_COUNT; seg++)
                {
                    max7219SendSymbol(seg, fontGetSpaceArray());
                }
                disconnectAnimFrame = 0;
            } else {
                max7219SendSymbol(disconnectAnimFrame, dot);
                disconnectAnimFrame++;
            }
        }
    }
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

/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */
static void wifiManagerConnectedCallback(void *pvParameter)
{
    ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    onWifiConnectionCallback(true);
    ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);
}

static void wifiManagerDisconnectedCallback(void *pvParameter)
{
    onWifiConnectionCallback(false);
}

static void onTimeSync(void)
{
    // Send time sync event to the queue
    EventType event = TIME_UPDATE_EVENT;
    timeSynced = true;
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
    /* start the wifi manager */
    wifi_manager_start();
    /* register a callback as an example to how you can integrate your code with the wifi manager */
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &wifiManagerConnectedCallback);
    wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, &wifiManagerDisconnectedCallback);
    // wifi_init(onWifiConnectionCallback);
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
                wifiConnected = true;
                obtain_time();
                xTimerChangePeriod(statusLedTimer, pdMS_TO_TICKS(2000), 0);
                ESP_LOGI(TAG, "Wi-Fi Connected");
                break;

            case WIFI_DISCONNECTED_EVENT:
                wifiConnected = false;
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
