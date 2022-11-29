#include "mqtt.hpp"
#include "header.hpp"
#include "carga.hpp"
#include "configmanager.hpp"

EventGroupHandle_t mqtt_event_group;
esp_mqtt_client_handle_t client;
esp_mqtt_client_config_t mqtt_cfg = {};
esp_mqtt_client_config_t mqtt_new_cfg = {};

extern CONFIGMAN *configmanagement;

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    char *my_json_input;

    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            xEventGroupSetBits( mqtt_event_group, MQTT_CONNECTED_BIT );

            msg_id = esp_mqtt_client_subscribe(client, "/divasdotiochico/writecfg", 0);
            ESP_LOGI(TAG_MQTT, "sent subscribe successful in %s, msg_id=%d", "/divasdotiochico/writecfg", msg_id);
            // msg_id = esp_mqtt_client_subscribe(client, "/divasdotiochico/qos1", 1);
            // ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

            // msg_id = esp_mqtt_client_unsubscribe(client, "/divasdotiochico/qos1");
            // ESP_LOGI(TAG_MQTT, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            xEventGroupClearBits( mqtt_event_group, MQTT_CONNECTED_BIT );
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/writecfg", "data", 0, 0, 0);
            ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            my_json_input = (char *)malloc(event->data_len + 1);
            // ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
            snprintf(my_json_input, event->data_len + 1, event->data);
            xTaskCreate(Task_MQTT_parse,"Task_MQTT_parse",4096,(void *)my_json_input,5,NULL);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}

void set_mqtt_uri(char *new_uri) {
    ESP_LOGI(TAG_MQTT, "Change MQTT Broker solicited: [%s]", new_uri);
    esp_mqtt_client_stop(client);
    mqtt_new_cfg.uri = (const char *)new_uri;

    if ((mqtt_Client_cert != NULL && mqtt_Client_key != NULL && mqtt_CAcert != NULL) && (strstr(new_uri, "mqtts") != NULL))  {
        mqtt_new_cfg.client_cert_pem = (const char *)mqtt_Client_cert;
        mqtt_new_cfg.client_key_pem = (const char *)mqtt_Client_key;
        mqtt_new_cfg.cert_pem = (const char *)mqtt_CAcert;
    }
    mqtt_cfg = mqtt_new_cfg;
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

const char* get_mqtt_uri(void) { return mqtt_uri2; }

/**
 * 
 * Inicia o MQTT
 */
void mqtt_app_start(void) {
    
    mqtt_cfg.uri = mqtt_uri;
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG_MQTT, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */
    mqtt_event_group = xEventGroupCreate();
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t) ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void Task_MQTT_parse(void *pvParm)
{
    char *data = (char *)pvParm;
//    ESP_LOGI(TAG_MQTT,"[MQTT_parse] Dados: %s", data);
    if (configmanagement->parse_json_data(data) < 0)
    {
        ESP_LOGE(TAG_MQTT,"[MQTT_parse] Erro no parse");
    }
    free(data);    
    vTaskDelete(NULL);
}