#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "cJSON.h"

#include <esp_http_server.h>

#include "httpServer.h"

static const char *TAG = "httpServer";

void wifi_write_nvs(char *ssid, char *pass){
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {
        nvs_set_str(my_handle, "SSID", ssid);
        nvs_set_str(my_handle, "PASS", pass);
        nvs_commit(my_handle);
        ESP_LOGI(TAG, "SSID: %s", ssid);
        ESP_LOGI(TAG, "PASS: %s", pass);
        esp_restart();
    }
        
}

static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);
        cJSON *SSID = cJSON_GetObjectItemCaseSensitive(json, "wifiSSID");
        cJSON *PASS = cJSON_GetObjectItemCaseSensitive(json, "wifiPASS");

        wifi_write_nvs(SSID->valuestring,PASS->valuestring);
    }

    /* Return JSON Message */
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Message", "Connected.");
    const char *returnJSON = cJSON_Print(root);
    httpd_resp_send_chunk(req, returnJSON, 150);
    free((void *)returnJSON);
    cJSON_Delete(root);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &echo);

    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}

void webserver_init(void){
    start_webserver();
}