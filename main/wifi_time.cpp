#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <wifi_time.hpp>
#include <time.h>
#include <sys/time.h>

static const char *TAG = "wifi_time";

void WifiTime::wifiEventHandler(void *pvParameter, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    WifiTime *pThis = (WifiTime *)pvParameter;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                pThis->wifi_is_connected = false;
                if (pThis->retry_num < WIFI_NR_RETRIES) {
                    esp_wifi_connect();
                    pThis->retry_num++;
                    ESP_LOGI(TAG, "retry %d", pThis->retry_num);
                } else {
                    pThis->retry_num = 0;
                    xEventGroupSetBits(pThis->wifi_event_group, WIFI_FAIL_BIT);
                }
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        pThis->retry_num = 0;
        xEventGroupSetBits(pThis->wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void WifiTime::monitorWifiTask(void *pvParameter) {
    WifiTime *pThis = (WifiTime *)pvParameter;
    ESP_ERROR_CHECK(esp_wifi_start());
    while (1) { pThis->monitorWifi(); }
}

void WifiTime::monitorWifi(void) {
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected: %s", WIFI_SSID);
        wifi_is_connected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed: %s", WIFI_SSID);
    }
    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    if (isTimeSet())
        vTaskDelay(WIFI_RECHECK_PERIOD_WITH_TIME_SYNC / portTICK_PERIOD_MS);
    else
        vTaskDelay(WIFI_RECHECK_PERIOD_NO_TIME_SYNC / portTICK_PERIOD_MS);
    esp_wifi_connect();
}

void WifiTime::initSTA(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, this, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, this, NULL));
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    xTaskCreate(monitorWifiTask, "monitor_wifi_task", 4096, this, 10, NULL);
}

void WifiTime::initSNTP(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    esp_sntp_init();
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}

void WifiTime::init(void) {
    initSTA();
    initSNTP();
}

bool WifiTime::isTimeSet(void) {
    time_t now; struct tm timeinfo;
    time(&now); localtime_r(&now, &timeinfo);
    return (timeinfo.tm_year > (1970 - 1900));
}

bool WifiTime::isWifiConnected(void) { return wifi_is_connected; }

void WifiTime::getTime(clock_time_t *t) {
    time_t now; struct tm timeinfo;
    time(&now); localtime_r(&now, &timeinfo);
    t->hour = (uint8_t)timeinfo.tm_hour;
    t->minute = (uint8_t)timeinfo.tm_min;
}
