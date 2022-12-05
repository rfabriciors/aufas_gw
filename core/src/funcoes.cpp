#include "header.hpp"
#include "mqtt.hpp"
#include "carga.hpp"
#include "configmanager.hpp"

RTC_DATA_ATTR int boot_count = 0;
struct tm timeinfo;
time_t now = 0;
bool led1_alterado = true;
CARGA *led1 = new CARGA(GPIO_NUM_33, (char *)"Led verde");
CARGA *ledModo = new CARGA(GPIO_NUM_23, (char *)"Manual/Automatic");
CONFIGMAN *configmanagement = new CONFIGMAN("settings.json");
bool sntp_initialized = false;

char *mqtt_uri = (char *)CONFIG_BROKER_URL;
char mqtt_uri2[128] = CONFIG_BROKER_URL;

EventBits_t uxBits; // Variável para monitorar a saída de xEventGroupWaitBits()
/*
EventGroupHandle_t wifi_event_group;
EventGroupHandle_t wifi_manager_event_group; // Deve-se tirar a palavra static de wifi_manager.h   XXXXXXXXXXXXXXXXXXX
*/
char buffer[BUFFER_RCP];
float saida;

/* brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event */
void cb_connection_ok(void *pvParameter)
{
	ESP_LOGI(TAG, "I have a connection!");
}

/*
    Essa função cria um servidor socket multi-client e deve ser chamada após
    ser estabelecido o endereçamento IP. Após a configuração inicial do dispositivo
    a uma rede WiFi, esse deverá ser resetando antes do promeiro uso

    1 - Cria um socket
    2 - Cria uma Task para cada cliente passando o número do socket como parâmetro
*/
void Task_Socket_Server(void *socket_handle) {

    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;

    // Cria o socket
    int rc;
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sock < 0) {
        ESP_LOGI(TAG,"socket: %d %s",sock, strerror(errno));
        goto END;
    }

    // Vincula o IP do servidor com a porta a ser aberta
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(CONFIG_DEFAULT_TCPIP_PORT);
    rc = bind(sock,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
    if(rc < 0) {
        ESP_LOGI(TAG, "bind: %d %s", rc, strerror(errno));
        goto END;
    }

    // Define o tamanho máximo da fila para atendimento das conexões dos clientes
    rc = listen(sock,MAX_NUMERO_CLIENTES);
    if(rc < 0) {
        ESP_LOGI(TAG, "listen: %d %s", rc, strerror(errno));
        goto END;
    }

    while(true) {
        socklen_t clientAddressLength = sizeof(clientAddress);
        // Aceita a conexão e retorna o socket do cliente
        int clientSock = accept(sock,(struct sockaddr *)&clientAddress, &clientAddressLength);
        if(clientSock < 0){
            ESP_LOGI(TAG, "accept %d %s",clientSock, strerror(errno));
        } else {
            ESP_LOGI(TAG, "SOCKET N: %d\n",clientSock);

            /*
                Cria uma task socket para cada cliente, passando como parâmetro o número do socket
                assim é possível conectar simultâneamente vários clientes e tratá-los individualmente
                É através de clientSock que se pode fazer o send e o read para os clientes
            */
           xTaskCreate(Task_Socket,"Task_Socket",12288,(void *)clientSock,5,NULL);
        }
    }
    END:
        // Se chegou aqui houve falha em algum ponto de conexão
        ESP_LOGI(TAG,"Fim da task Socket_Server");
        vTaskDelete(NULL);
}

/*
    Essa task é gerada para cada cliente conectada. Ela permite que o cliente mande 
    comandos para o ESP e esse interpreta esses comandos e responde de acordo com
    a rotina programada. Cada cliente é independente dos demais.
*/
void Task_Socket(void *socket_handle) {
    int clientSocket = (int)socket_handle;
    char *data = (char *)malloc(BUFFER_RCP);
    while(true) {
        ssize_t sizeRead = recv(clientSocket, data, BUFFER_RCP, 0);
        if(sizeRead < 0){
            ESP_LOGI(TAG, "recv: %d %s",sizeRead, strerror(errno));
            break;
        }
        if(sizeRead == 0){
            break;
        }
//        ESP_LOGI(TAG,"Task_Socket: Socket: %d - Dados lidos (tamanho: %d): %.*s",clientSocket,sizeRead,sizeRead,data);
       setValor(data, sizeRead, clientSocket);
       vTaskDelay(200/portTICK_PERIOD_MS);
    }
    free(data);    
    close(clientSocket);
    ESP_LOGI(TAG,"Finalizando o socket: %d", clientSocket);
    vTaskDelete(NULL);
}

void Task_Core(void *socket_handle) {
    // Atualiza a hora via NTP
    if (sntp_initialized != true)
    {
        if (timeinfo.tm_year < (2016 - 1900))
        {
            ESP_LOGI(SNTP_TAG, "Time not sync yet, getting time by NTP protocol");
            if (!obtain_time())
            {
                ESP_LOGE(SNTP_TAG, "Cannot get time, using default timestamp");
            }
            time(&now);
        }
        setenv("TZ", "UTC+3",1);
        tzset();
        localtime_r(&now, &timeinfo);
        ESP_LOGI(SNTP_TAG, "Current date and time: %d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    int i = 0;
    while (mqtt_uri[i] != '\0')
    {
        mqtt_uri2[i] = mqtt_uri[i];
        i++;
    }
    mqtt_uri2[i] = '\0';

    mqtt_app_start();

    //  Inicia a task de envio de dados MQTT
    if (xTaskCreate(Task_SendMQTT, "Task_SendMQTT", 8192, NULL, 5, NULL) != pdTRUE)
    {
        ESP_LOGE(TAG, "Nao foi possivel criar a task \'Task_SendMQTT\'");
        return;
    }

    //  Inicia a task button
    // if (xTaskCreate(Task_button, "Task_button", 4098, NULL, 5, NULL) != pdTRUE)
    // {
    //     ESP_LOGE(TAG, "error - nao foi possivel alocar task_button.\n");
    //     return;
    // }
    startconfig();
    while(1){
            vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void startconfig(void){
    // Load data from flash
    esp_err_t err;
    char running_config[BUFFER_RCP];
    int j = 0;
    while (j < BUFFER_RCP)
    {
        running_config[j] = '\0';
        j++;
    }
    err = configmanagement->load_data(running_config);
    if (err != ESP_OK)
        return;
    configmanagement->parse_json_data(running_config);
}

void setValor(char *str,int size, int clientSocket){
    int i = 0, j = 0, l = 0, contchar = 0;
    char valor[BUFFER_RCP];
    char delim[] = ";";
    char buffer_envio[BUFFER_RCP];
    for (int m = 0; m < BUFFER_RCP;m++){
        buffer_envio[m] ='\0';
    }
    char comando[20];

    for (i = 0; i < BUFFER_RCP; i++)
    {
        valor[i] = '\0';
    }
    for (i = 0; i < 20; i++)
    {
        comando[i] = '\0';
    }
    for (i = 0;  i < size;  i++){   
        if (str[i] == ';') {
            contchar++;
        }    
    }
    ESP_LOGI(TAG_TASK_SOCKET, "setValor: size %d, Delimitadores %d", size, contchar);
    size = size - contchar;
    char *ptr = strtok(str,delim);
    while ((ptr != NULL) && (l < (size-2)))
    {
        i=0;

        while ((ptr[i] != '$') && (i < strlen(ptr)))
        {
            comando[i] = ptr[i];
            i++;
        }
        while (i < strlen(ptr))
        {
            if(ptr[i]=='$'){
                i++;
                continue;
            }
            valor[j] = ptr[i];
            j++;
            i++;  
        }
        if (strcmp(comando, "saveconfig") == 0)
        {
            configmanagement->saveconfig();
        }
        if (strcmp(comando, "loadconfig") == 0){
            startconfig();
        }
        if (strcmp(comando, "getstartupconfig") == 0)
        {
            char *data_receive = new char[BUFFER_RCP]();
            configmanagement->load_data(data_receive);
            enviaDados(clientSocket, data_receive);
            delete data_receive;
        }
        if (strcmp(comando, "erasestartupconfig") == 0)
        {
            esp_err_t err;
            err = configmanagement->erase_data();
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG_TASK_SOCKET, "Erro ao limpar dados [%s]\n", esp_err_to_name(err));
                sprintf(buffer_envio, "erase_data$Err");
            }
            else
            {
                sprintf(buffer_envio, "erase_data$Ok");
            }
            enviaDados(clientSocket, buffer_envio);
        }
        if (strcmp(comando, "setmqtturi") == 0)
        {
            sprintf(buffer_envio, "%s", valor);
            set_mqtt_uri(buffer_envio);
        }
        if (strcmp(comando, "getmqtturi") == 0)
        {
            sprintf(buffer_envio, "mqtturi$%s", get_mqtt_uri());
            enviaDados(clientSocket, buffer_envio);
        }

            ptr = strtok(NULL, delim); // deve sempre ser o último comando da iteração
    }
}

/*
    Essa função envia dados para um socket informado
*/
void enviaDados(int clientSocket,char *data) {
    strcat(data, TERMINATOR);
    int rc = send(clientSocket,data,strlen(data),0);
    if(rc < 0) {
        ESP_LOGE(TAG, "send: %d %s", rc, strerror(errno));
    }
    //#ifdef DEBUG
         ESP_LOGI(TAG,"enviaDados: send: rc: %d size: %d data: %s", rc, strlen(data),data);
    //#endif
}

void Task_LED(void *pVparam) {
    #ifdef DEBUG
        ESP_LOGI( TAG, "Task_LED inited" );
    #endif
    gpio_config_t led_cfg = {};
        led_cfg.pin_bit_mask = GPIO_OUTPUT;
        led_cfg.mode = GPIO_MODE_OUTPUT;
        led_cfg.intr_type = GPIO_INTR_DISABLE;

    int cnt = 0;
    gpio_config(&led_cfg);
    while (true) {
        time(&now);
        localtime_r(&now, &timeinfo);
        uxBits = xEventGroupGetBits( wifi_manager_event_group );
        if ((uxBits & BIT0) != 0 )
  		{
            gpio_set_level(LED_BUILDING, 1);
            vTaskDelay(ATRASO_50/portTICK_PERIOD_MS);
            gpio_set_level(LED_BUILDING, 0);
            vTaskDelay(ATRASO_50/portTICK_PERIOD_MS);
            gpio_set_level(LED_BUILDING, 1);
            vTaskDelay(ATRASO_50/portTICK_PERIOD_MS);
            gpio_set_level(LED_BUILDING, 0);
            vTaskDelay(ATRASO_2850/portTICK_PERIOD_MS);
  		} else {
            gpio_set_level(LED_BUILDING, cnt%2);
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        cnt++;
    }
}

void Task_SendMQTT(void *pVparam)
{
#ifdef DEBUG
    ESP_LOGI(TAG, "Task_SendMQTT initialized");
#endif
    EventBits_t staBits;
    char buffer_envio[BUFFER_RCP];
    char *inforeceived = buffer_envio;
    staBits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    while(1) {
        if( (staBits & MQTT_CONNECTED_BIT ) != 0 ) {
            configmanagement->getdata(MQTT_CONFIG_MESSAGE, inforeceived);
            esp_mqtt_client_publish(client, "/divasdotiochico/infosystem", buffer_envio, 0, 0, 0);
        }

        vTaskDelay(700 / portTICK_PERIOD_MS);
    }
}

void initialize_sntp(void)
{
    #ifdef DEBUG
        ESP_LOGI(SNTP_TAG, "Initializing SNTP");
    #endif
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, CONFIG_DEFAULT_NTP_URL);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    sntp_initialized = true;
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(SNTP_TAG, "Update time notification...");
}

bool obtain_time(void) {
    initialize_sntp();
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    memset(&timeinfo, 0, sizeof(struct tm));
    int retry = 0;
    const int retry_count = CONFIG_DEFAULT_NTP_CONNECT_ATTEMPS;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry <= retry_count)
    {
        ESP_LOGI(SNTP_TAG, "Waiting define system time.. (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(esp_task_wdt_reset());
    }
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
    return timeinfo.tm_year > (2016 - 1900);
}

// void Task_button(void *pvParameter)
// {
//     int aux = 0;

// #ifdef DEBUG
//     ESP_LOGI(TAG, "Task_button init");
// #endif
//     gpio_config_t input_cfg = {};
//     input_cfg.pin_bit_mask = GPIO_INPUT;
//     input_cfg.mode = GPIO_MODE_INPUT;
//     input_cfg.intr_type = GPIO_INTR_DISABLE;
//     input_cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
//     gpio_config(&input_cfg);

//     for (;;)
//     {
//         /**
//          * Botão motor pressionado?
//          */
//         if (gpio_get_level(BUTTON_MOTOR) == 1 && aux == 0)
//         {
//             /**
//              * Aguarda 80ms devido ao bounce;
//              */
//             vTaskDelay(80 / portTICK_PERIOD_MS);

//             if (gpio_get_level(BUTTON_MOTOR) == 1 && aux == 0)
//             {
//                 if (motor->getCarga() == 0)
//                 {
//                     motor->switchCarga(1);
//                 } else {
//                     motor->switchCarga(0);
//                 }
//                 aux = 1;
//             }

//         }
//         /**
//          * Botão solto?
//          */
//         if (gpio_get_level(BUTTON_MOTOR) == 0 && aux == 1)
//         {
//             vTaskDelay(80 / portTICK_PERIOD_MS);

//             if (gpio_get_level(BUTTON_MOTOR) == 0 && aux == 1)
//             {
//                 aux = 0;
//             }
//         }

//         vTaskDelay(10 / portTICK_PERIOD_MS);
//     }
// }