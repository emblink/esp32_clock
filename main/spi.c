#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

/* Defining pins for ESP32 which uses MISO, MOSI, CS, SCLK */
#define ESP_HOST     SPI2_HOST 
#define PIN_NUM_MISO GPIO_NUM_NC                          
#define PIN_NUM_MOSI GPIO_NUM_6                          
#define PIN_NUM_CLK  GPIO_NUM_4                          
#define PIN_NUM_CS   GPIO_NUM_7 

/* Declaring the funtions which are used in the program */
#define SPI_TAG "spi_protocol"
static spi_device_handle_t spi;

void vSpiInit(void)
{
    esp_err_t ret;

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = GPIO_NUM_NC,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 512, // Maximum transfer size in bytes
    };

    // Configure SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000, // 8 MHz
        .mode = 0,                         // SPI mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = PIN_NUM_CS,        // Chip Select pin
        .queue_size = 7,                   // Queue 7 transactions
    };

    // Initialize SPI bus
    ret = spi_bus_initialize(ESP_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // Add device to SPI bus
    ret = spi_bus_add_device(ESP_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(SPI_TAG, "SPI initialized successfully");
}

void spiSendData(const uint8_t data[], uint16_t len)
{
    esp_err_t ret;

    // Configure SPI transaction
    spi_transaction_t trans_desc = {
        .tx_buffer = (const void *)data, // Use tx_buffer for larger data
        .length = len * 8,               // Length in bits
    };

    // Perform SPI transaction
    ret = spi_device_polling_transmit(spi, &trans_desc);
    if (ret == ESP_OK)
    {
        // ESP_LOGI(SPI_TAG, "Data written successfully");
    }
    else
    {
        ESP_LOGE(SPI_TAG, "SPI write operation failed");
    }
}