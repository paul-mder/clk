#ifndef _INCLUDE_CLOCK_MACHINE_HPP_
#define _INCLUDE_CLOCK_MACHINE_HPP_

#include "clock_machine_states.hpp"
#include "rotary_encoder.hpp"
#include "wifi_time.hpp"
#include "display.hpp"
#include "esp_http_server.h"

#define NVS_STORAGE          "storage"
#define NVS_ALARM_HOUR       "alarm_hour"
#define NVS_ALARM_MINUTE     "alarm_minute"

class ClockState;

class ClockMachine {
  public:
    ClockMachine(RotaryEncoder* encoder_ref);
    void saveAlarmTimeInNVS();
    void setState(ClockState& newState);
    clock_time_t getTimeToAlarm(clock_time_t current_time, clock_time_t alarm_time);
    WifiTime* getWifiTime();
    Display* getDisplay();
    RotaryEncoder* getEncoder();
    void triggerTimer(uint16_t timer_ms);
    void checkWifiStatus(bool force_update);
    void run();
    void buttonShortPressed();
    void buttonLongPressed();
    void encoderRotated(rotary_encoder_pos_t position, rotary_encoder_dir_t direction);
    void wakeDisplay();
    ~ClockMachine();

    clock_time_t stored_time;
    bool is_alarm_set = false;
    clock_time_t alarm_time;
    bool time_has_changed;

    struct {
        uint16_t snooze_time_s = 300;
    } settings;

  private:
    esp_err_t readNVSValues();
    void writeNVSDefaultValues();
    void checkTimeUpdate(void);
    void startWebServer(void);
    static esp_err_t handleRoot(httpd_req_t *req);
    static esp_err_t handleSetAlarm(httpd_req_t *req);
    static esp_err_t handleGetStatus(httpd_req_t *req);

    ClockState* state;
    WifiTime wifi_time;
    Display display;
    RotaryEncoder* encoder;
    int64_t active_timer_us = 0;
    int64_t trigger_timestamp_us = 0;
    bool last_wifi_connected_status;
    httpd_handle_t server = NULL;

    // Display-Timeout
    int64_t last_interaction_us = 0;
    bool display_is_off = false;
    bool next_input_is_wakeup = false;
};

#endif /* _INCLUDE_CLOCK_MACHINE_HPP_ */
