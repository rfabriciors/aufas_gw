#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <errno.h>
#include "esp_system.h"
#include "configmanager.hpp"
#include <esp_ota_ops.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#define TAG_CONFIGMAN "CONFIGMAN"
#define STORAGE_NAMESPACE "contaut"

extern struct tm timeinfo;
extern time_t now;
const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

CONFIGMAN::CONFIGMAN(const char *nspace)
{
    this->nameSpace = (char *)nspace;
}

/*
    Modelo:
    {"type":"setconfig","led":[{"name":"led_1","activate":0,"location":"engine"}]}
*/

int CONFIGMAN::parse_json_data(char *buffer)
{
    cJSON *root, *item, *value_item, *aux;
    char *value_str = NULL;
#ifdef CONFIG_DEBUG_MODE
    ESP_LOGI(TAG_CONFIGMAN, "Analising data: %s", buffer);
#endif
    root = cJSON_Parse((char *)buffer);
    if (!root)
    {
        ESP_LOGE(TAG_CONFIGMAN, "JSON struct error: [%s]", cJSON_GetErrorPtr());
        return -1;
    }
    int json_item_num = cJSON_GetArraySize(root);
#ifdef CONFIG_DEBUG_MODE
    ESP_LOGI(TAG_CONFIGMAN, "Total itens in data receved (root): %d", json_item_num);
#endif
    for (int32_t i = 0; i < json_item_num; ++i)
    {                                       // percorre os itens na raiz
        item = cJSON_GetArrayItem(root, i); // obtém os subitens do item 'i'
        if (!item)
        { // parse item failed, reponse error code : -i * 100
            ESP_LOGE(TAG_CONFIGMAN, "Parse item error");
            break;
        }
        if (0 == strncmp(item->string, "type", sizeof("type")))
        {
            value_str = item->valuestring;
        //    ESP_LOGI(TAG_CONFIGMAN, "value_str: %s", value_str);
        }
        else if (0 == strncmp(item->string, "led", sizeof("led")))
        {
            // Obter a quantidade de vetores (número de instâncias Motor)
            int8_t instLEDNum = cJSON_GetArraySize(item);
            for (int8_t k = 0; k < instLEDNum; k++)
            {
                value_item = cJSON_GetArrayItem(item, k);
                aux = cJSON_GetObjectItem(value_item, "name");
                if (aux != NULL)
                {
                    if (0 == strncmp(aux->valuestring, "led_1", sizeof("led_1")))
                    {
                        (cJSON_GetObjectItem(value_item, "activate") != NULL) ? led1->switchCarga((int)cJSON_GetObjectItem(value_item, "activate")->valueint) : NULL;
                        (cJSON_GetObjectItem(value_item, "location") != NULL) ? led1->setLocation((char *)cJSON_GetObjectItem(value_item, "location")->valuestring) : NULL;
                        led1_alterado = true;
                    }
                }
            }
        }
        else if (0 == strncmp(item->string, "mqtt", sizeof("mqtt")))
        {
            set_mqtt_uri(cJSON_GetObjectItem(item, "mqtt_uri")->valuestring);
#ifdef CONFIG_DEBUG_MODE
            ESP_LOGI(TAG_CONFIGMAN, "mqtt_uri: %s", cJSON_GetObjectItem(item, "mqtt_uri")->valuestring);
            ESP_LOGI(TAG_CONFIGMAN, "mqtt_subscribe_topic: %s", cJSON_GetObjectItem(item, "mqtt_subscribe_topic")->valuestring);
            ESP_LOGI(TAG_CONFIGMAN, "mqtt_publish_topic: %s", cJSON_GetObjectItem(item, "mqtt_publish_topic")->valuestring);
#endif
        }
        else if (0 == strncmp(item->string, "timestamp", sizeof("timestamp")))
        {
            ESP_LOGI(TAG_CONFIGMAN, "Timestamp saved data: %s", item->valuestring);
        }
        else
        {
            ESP_LOGE(TAG_CONFIGMAN, "cannot parse JSON Item:%d", i);
            break;
        }
    }
    free(value_str);
    return 1;
    }

void CONFIGMAN::getdata(int datatype, char *buffer)
{
    // #define MQTT_CONFIG_MESSAGE 0
    // #define STORE_SYSTEM_CONFIG 1
    time(&now);
    char timestamp[72];
    sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    esp_chip_info_t chip_info;
    cJSON *root, *ledArray, *led_1, *systemInfo, *mqtt;
    char *json_info;
    root = cJSON_CreateObject();
    led_1 = cJSON_CreateObject();
    ledArray = cJSON_CreateArray();
    mqtt = cJSON_CreateObject();

    if (datatype == STORE_SYSTEM_CONFIG)
    {
        cJSON_AddStringToObject(root, "type", "setconfig");
        cJSON_AddStringToObject(root, "timestamp", timestamp);

        cJSON_AddStringToObject(mqtt, "mqtt_uri", get_mqtt_uri());
        cJSON_AddStringToObject(mqtt, "mqtt_subscribe_topic", get_mqtt_subscribe());
        cJSON_AddStringToObject(mqtt, "mqtt_publish_topic", get_mqtt_publish());
        cJSON_AddItemToObject(root, "mqtt", mqtt);
    }
    else if (datatype == MQTT_CONFIG_MESSAGE)
    {
        const esp_app_desc_t *app_desc = esp_ota_get_app_description();
        cJSON_AddStringToObject(root, "type", "systemconfig");
        systemInfo = cJSON_CreateObject();
        esp_chip_info(&chip_info);
        cJSON_AddNumberToObject(systemInfo, "uptime", esp_timer_get_time() / 1000000);
        cJSON_AddNumberToObject(systemInfo, "freeheap", esp_get_free_heap_size());
        cJSON_AddNumberToObject(systemInfo, "cpucores", chip_info.cores);
        cJSON_AddStringToObject(systemInfo, "BT", (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "");
        cJSON_AddStringToObject(systemInfo, "BLE", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
        cJSON_AddNumberToObject(systemInfo, "revision", chip_info.revision);
        cJSON_AddNumberToObject(systemInfo, "flashsize", spi_flash_get_chip_size() / (1024 * 1024));
        cJSON_AddStringToObject(systemInfo, "appversion", app_desc->version);
        cJSON_AddItemToObject(root, "systeminfo", systemInfo);
    }

    cJSON_AddStringToObject(led_1, "name", "led_1");
    cJSON_AddNumberToObject(led_1, "activate", led1->getCarga());
    cJSON_AddStringToObject(led_1, "location", led1->getLocation());
    cJSON_AddItemToArray(ledArray, led_1);
    cJSON_AddItemToObject(root, "led", ledArray);

    json_info = cJSON_PrintUnformatted(root);

    sprintf(buffer, json_info);
    cJSON_free(json_info);
    cJSON_Delete(root);
}

esp_err_t CONFIGMAN::saveconfig(void)
{
    char *data = new char[1024]();
    int size_data=0;
    esp_err_t err;
    this->getdata(STORE_SYSTEM_CONFIG, data);
    while (data[size_data] != '\0')
    {
        size_data++;
    }
#ifdef CONFIG_DEBUG_MODE
    ESP_LOGI(TAG_CONFIGMAN, "Data read: %s", data);
#endif
    err = this->save_data(data, size_data);
    if (err != ESP_OK)
        ESP_LOGE(TAG_CONFIGMAN, "Cannot save data\n");
    return ESP_OK;
}

esp_err_t CONFIGMAN::save_data(char *data, int size)
{
    nvs_handle_t settings_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &settings_handle);
    if (err != ESP_OK) return err;

    err = nvs_set_str(settings_handle, this->nameSpace, data);
    if (err != ESP_OK)
        return err;
    free(data);

    err = nvs_commit(settings_handle);
    if (err != ESP_OK)
        return err;

    nvs_close(settings_handle);
    return ESP_OK;
}

esp_err_t CONFIGMAN::load_data(char *data)
{
    nvs_handle_t settings_handle;
    esp_err_t err;
    size_t len;
    char *aux = new char[1024]();

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &settings_handle);
    if (err != ESP_OK){
        ESP_LOGE(TAG_CONFIGMAN, "Error in open flash [%s]\n", esp_err_to_name(err));
        nvs_close(settings_handle);
        return err;
    }
        
    err = nvs_get_str(settings_handle, this->nameSpace, NULL, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_CONFIGMAN, "found [%s]\n", esp_err_to_name(err));
        len = 0;
        this->getdata(STORE_SYSTEM_CONFIG, aux);
        nvs_close(settings_handle);
        return err;
    }

    if (len > 0)
    {
        err = nvs_get_str(settings_handle, this->nameSpace, aux, &len);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG_CONFIGMAN, "Erro to read data in flash\n");
            nvs_close(settings_handle);
            return err;
        }
    }

    int i = 0;
    while (aux[i] != '\0')
    {
        data[i] = aux[i];
        i++;
    }
    delete[] aux;
    // Close
    nvs_close(settings_handle);
    return ESP_OK;
    }

esp_err_t CONFIGMAN::erase_data(void)
{
    nvs_handle_t settings_handle;
    esp_err_t err;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &settings_handle);
    if (err != ESP_OK)
        ESP_LOGE(TAG_CONFIGMAN, "Erro ao abrir a flash\n");
    err = nvs_erase_key(settings_handle, this->nameSpace);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_CONFIGMAN, "Erro ao limpar configurações da flash\n");
        return err;
    }
    return ESP_OK;
}