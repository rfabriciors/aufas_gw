
#pragma once
#ifndef __CARGA_HPP_
#define __CARGA_HPP_
#include "driver/gpio.h"

class CARGA {
    private:
        int status;
        char *location;
        gpio_num_t cargaOutput;
        gpio_config_t carga_cfg = {};

    public:
        CARGA(gpio_num_t portCARGA, char *location_);
        char *getLocation(void);
        int setLocation(char *location_);
        void switchOn(void);
        void switchOff(void);
        int switchCarga(int);
        int getCarga(void);
};

#endif