#ifndef __WIFIMAN_HELPER_H
#define __WIFIMAN_HELPER_H

#include "esp_wifi.h"
#include "types.h"

esp_err_t _abaWifimanNVSInit(AbaWifiman *app);

esp_err_t _abaWifimanNVSGetAPConfig(AbaWifiman *app, wifi_config_t *wifi_cfg);

esp_err_t _abaWifimanNVSGetSTAConfig(AbaWifiman *app, wifi_config_t *wifi_cfg);

esp_err_t _abaWifimanWiFiInit(AbaWifiman *app);

esp_err_t _abaWifimanWiFiDeinit();

#endif