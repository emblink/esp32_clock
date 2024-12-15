#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"
#include "sdkconfig.h"
#include "sntp.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

#define CONFIG_SNTP_TIME_SERVER "pool.ntp.org"

static const char *TAG = "sntp";
static TimeSyncCallback timeSyncCb = NULL;

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    // Set timezone to Eastern Standard Time and print local time
    char strftime_buf[64];
    time_t now;
    struct tm timeinfo;
    // Set the timezone to EST
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    // Get the current time
    time(&now);
    localtime_r(&now, &timeinfo);
    // Format and print the time
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);
    timeSyncCb();
}

static void print_servers(void)
{
    ESP_LOGI(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (esp_sntp_getservername(i)){
            ESP_LOGI(TAG, "server %d: %s", i, esp_sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = esp_sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(TAG, "server %d: %s", i, buff);
        }
    }
}

void sntpInit(TimeSyncCallback onSyncCb)
{
    ESP_LOGI(TAG, "Initializing and starting SNTP");
    timeSyncCb = onSyncCb;
    /*
     * This is the basic default config with one server and starting the service
     */
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
    config.sync_cb = time_sync_notification_cb;     // Note: This is only needed if we want
    //config.smooth_sync = true;
    esp_netif_sntp_init(&config);
    print_servers();
}

void obtain_time(void)
{
    esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS);
}

Time getTime(void)
{
    time_t now;
    struct tm timeinfo;
        // Get the current time
    time(&now);
    localtime_r(&now, &timeinfo);
    Time t = {0};
    t.hour = timeinfo.tm_hour;
    t.min = timeinfo.tm_min;
    t.sec = timeinfo.tm_sec;
    return t;
}