#pragma once
#include <stdint.h>

void spiSendData(const uint8_t data[], uint16_t len);
void vSpiInit(void);