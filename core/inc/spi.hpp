#pragma once
#ifndef _SPI_HEADER_HPP_
#define _SPI_HEADER_HPP_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/spi_master.h>
#include <errno.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"

#define MISO_PORT GPIO_NUM_12   // pin 4 Esquerda
#define SCLK_PORT GPIO_NUM_13   // pin 3 Esquerda
#define MOSI_PORT GPIO_NUM_14   // pin 5 Esquerda (Não usado)
#define CS_PORT GPIO_NUM_33     // pin 6 Esquerda
// #define SCLK_PORT GPIO_NUM_16 // pin 6 Direita (Fio azul) -> CS de outro dispositivo

#define TERMOPAR_CAP_DELAY 200
#define CELSIUS 0
#define FAHRENHEIT 1
#define KELVIN 2
#define TAG_SPI "spi"
#define DEBUG   1

class SPI {
    public:
        SPI(int miso_port, int mosi_port, int clk_port);
};

class THERM {
    private:
        float therm_read;
        float offset;
        unsigned int unit;
        spi_device_handle_t spi_dev_handle;

    public:
        THERM(int cs_port, int clk, int unit);
        float spi_therm_read(void);
        float setOffset(float offset);
        float getOffset(void);
        int getUnit(void);
        int setUnit(unsigned int);
};


extern float temp_therm_1;
extern QueueHandle_t xQueueTemp1;

#endif