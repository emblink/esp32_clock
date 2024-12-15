#pragma once
#include <stdint.h>

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t month;
    uint8_t day;
    uint16_t year;
} Time;

typedef void (*TimeSyncCallback)(void);

void sntpInit(TimeSyncCallback onSyncCb);
void obtain_time(void);
Time getTime(void);
