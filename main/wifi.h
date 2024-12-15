#pragma once
#include <stdbool.h>

typedef void (*WifiConnectionCallback)(bool);

void wifi_init(WifiConnectionCallback onConnectionCb);
bool is_wifi_connected();