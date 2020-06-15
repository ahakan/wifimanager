#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "softAP.h"
#include "httpServer.h"
#include "wifi.h"

static const char *TAG = "main";

void app_main(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
    webserver_init();

    /*  Read Non-volatile Storage 
        SSID = Wifi SSID
        SSID size 32
        PASS = Wifi PASS
        PASS size 64        */
    nvs_handle_t my_handle;
    char* SSID;
    char* PASS;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {

        size_t required_size_ID=32;
        nvs_get_str(my_handle, "SSID", NULL, &required_size_ID);
        SSID = malloc(required_size_ID);
        nvs_get_str(my_handle, "SSID", SSID, &required_size_ID);

        size_t required_size_PASS=64;
        nvs_get_str(my_handle, "PASS", NULL, &required_size_PASS);
        PASS = malloc(required_size_PASS);
        nvs_get_str(my_handle, "PASS", PASS, &required_size_PASS);

        ESP_LOGI(TAG, "SSID: %s", SSID);
        ESP_LOGI(TAG, "PASS: %s", PASS);
    }

}