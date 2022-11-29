#pragma once
#ifndef __MQTT_H_
#define __MQTT_H_

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
#include "mqtt_client.h"
#include "cJSON.h"

#define TAG_MQTT "MQTT"

/* @brief indicate that the ESP32 is currently connected. */
const int MQTT_CONNECTED_BIT = BIT0;
static const char mqtt_Client_cert[] = R"(
)";
static const char mqtt_Client_key[] = R"(
)";
static const char mqtt_CAcert[] = R"(
)";
extern EventGroupHandle_t mqtt_event_group;
extern esp_mqtt_client_handle_t client;
extern char *mqtt_uri;
extern char mqtt_uri2[128];
void Task_MQTT_parse(void *pvParm);
void mqtt_app_start(void);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
void set_mqtt_uri(char *);
const char *get_mqtt_uri(void);


#endif