#ifndef _INCLUDE_WIFI_TIME_HPP_
#define _INCLUDE_WIFI_TIME_HPP_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "clock_common.hpp"
#include "mqtt_config.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_RECHECK_PERIOD_WITH_TIME_SYNC 900000
#define WIFI_RECHECK_PERIOD_NO_TIME_SYNC   15000
#define WIFI_NR_RETRIES 3

class WifiTime {
    static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void monitorWifiTask(void *pvParameter);
    void monitorWifi(void);
    void initSTA(void);
    void initSNTP(void);

    EventGroupHandle_t wifi_event_group;
    bool wifi_is_connected = false;
    uint8_t retry_num = 0;

   public:
    void init(void);
    bool isTimeSet(void);
    bool isWifiConnected(void);
    void getTime(clock_time_t *time);
};

#endif
