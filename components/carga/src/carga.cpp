#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <errno.h>
#include "carga.hpp"
#include "esp_system.h"

#define CARGA_TAG "CARGA"

CARGA::CARGA(gpio_num_t portCarga, char *location_)
{
    uint64_t GPIO_OUTPUT = (1ULL << portCarga);

    carga_cfg.pin_bit_mask = GPIO_OUTPUT;
    carga_cfg.mode = GPIO_MODE_OUTPUT;
    carga_cfg.intr_type = GPIO_INTR_DISABLE;
    this->cargaOutput = portCarga;
    gpio_config(&carga_cfg);
    gpio_set_level(portCarga, 0);
    this->status = false;
    this->location = location_;
}

int CARGA::getCarga(void) { return this->status; }
int CARGA::setLocation(char *location_)
{
    this->location = location_;
    return 0;
}
char *CARGA::getLocation(void) { return this->location; }

void CARGA::switchOn(void)
{
    gpio_set_level(this->cargaOutput, 1);
    this->status = 1;
#ifdef CONFIG_DEBUG_MODE
    ESP_LOGI(CARGA_TAG, "Load %s turn on", this->location);
#endif
}

void CARGA::switchOff(void)
{
    gpio_set_level(this->cargaOutput, 0);
    this->status = 0;
#ifdef CONFIG_DEBUG_MODE
    ESP_LOGI(CARGA_TAG, "Load %s turn off", this->location);
#endif
}

int CARGA::switchCarga(int input)
{
    if (input == 1) {
        this->switchOn();
    } else if (input == 0) {
        this->switchOff();
    }
    return this->status;
}