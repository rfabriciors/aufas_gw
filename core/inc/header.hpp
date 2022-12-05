#pragma once
#ifndef _HEADER_HPP_
#define _HEADER_HPP_

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <netinet/in.h>
#include <errno.h>
#include "driver/ledc.h"
#include "wifi_manager.h"
#include "sys/reent.h"
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include "cJSON.h"

#define MAX_NUMERO_CLIENTES CONFIG_DEFAULT_AP_MAX_CONNECTIONS
#define LED_BUILDING GPIO_NUM_26
#define LED1 GPIO_NUM_33
#define GPIO_OUTPUT (1ULL << LED_BUILDING)
#define BUTTON_MAN_AUT GPIO_NUM_27
#define BUTTON_MOTOR GPIO_NUM_16
#define GPIO_INPUT ((1ULL << BUTTON_MAN_AUT) | (1ULL << BUTTON_MOTOR))
#define ATRASO_50 50
#define ATRASO_2850 2850
#define BUFFER_RCP 3072

#define MEMORY_TAG "Memory"
#define SNTP_TAG "sntp"
#define TAG_TASK_SOCKET "socket"
#define TAG "AUFA_GW"
#define TERMINATOR ";"
extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t wifi_manager_event_group; // Deve-se tirar a palavra static de wifi_manager.h   XXXXXXXXXXXXXXXXXXX

void monitoring_task(void *pvParameter);
void getdata(int datatype, char* buffer);
void setValor(char *str, int size, int clientSocket);
void enviaDados(int clientSocket,char *data);
void Task_Core(void *socket_handle);
void Task_Socket_Server(void *socket_handle);
void Task_Socket(void *socket_handle);
void Task_LED(void *pVparam);
void Task_SendMQTT(void *pVparam);
void Task_SPI(void *pvParm);
//void Task_button(void *pvParameter);
void time_sync_notification_cb(struct timeval *tv);
bool obtain_time(void);
void initialize_sntp(void);
void startconfig(void);

#endif