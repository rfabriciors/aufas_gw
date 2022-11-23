#include "header.hpp"
#include "mqtt.hpp"

using namespace std;

extern "C" {
	void app_main();
}

void app_main()
{
	esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Esse é o ESP32 com %d CPU cores, WiFi%s%s\n", chip_info.cores, (chip_info.features & CHIP_FEATURE_BT)?"/BT":"",
            (chip_info.features & CHIP_FEATURE_BLE)?"/BLE":"");
    printf("silicon revision %d, ",chip_info.revision);
    printf("%dMB %s flash\n",spi_flash_get_chip_size()/(1024*1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH)?"embedded":"external");

	wifi_manager_start();

	if (xTaskCreate(Task_LED, "Task_LED", 2048, NULL, 1, NULL) != pdTRUE)
	{
		ESP_LOGE(TAG, "Nao foi possivel ciar a task \'Task_LED\'");
		return;
	}

	if (xTaskCreate(Task_Core, "Task_Core", 10000, NULL, 1, NULL) != pdTRUE)
	{
		ESP_LOGE(TAG, "Nao foi possivel ciar a task \'Task_CORE\'");
		return;
	}
	/* registra a função de callback que deverá ser executada quando o wifi manager terminar as configurações*/
	// wifi_manager_set_callback(EVENT_STA_GOT_IP, &Task_Core);
	wifi_manager_set_callback(EVENT_STA_GOT_IP, &Task_Socket_Server);
}
