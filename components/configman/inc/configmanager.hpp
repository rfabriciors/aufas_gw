#pragma once
#ifndef __CONFIGMAN_H_
#define __CONFIGMAN_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include <freertos/event_groups.h>
#include "esp_log.h"
#include "cJSON.h"
#include "mqtt.hpp"
#include "header.hpp"
#include "carga.hpp"

#define MQTT_CONFIG_MESSAGE 0
#define STORE_SYSTEM_CONFIG 1
#define CONTROL_MODE_MANUAL 0
#define CONTROL_MODE_AUTOMATIC 1
#define CONTROL_MODE_TUNNING 2

extern CARGA *led1;
extern bool pid_alterado;
extern bool led1_alterado;
extern esp_mqtt_client_config_t mqtt_cfg;

class CONFIGMAN
{
private:
    char *nameSpace;
    esp_err_t save_data(char *, int size);

public:
    CONFIGMAN(const char *nspace);
    esp_err_t load_data(char *);
    esp_err_t erase_data(void);
    int parse_json_data(char *);
    void getdata(int, char *);
    esp_err_t saveconfig(void);
    void switchcontrolmode(void);
};

#endif