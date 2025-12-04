#ifndef __WIFIMAN_EVENT_HANDLE_H
#define __WIFIMAN_EVENT_HANDLE_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "aba/log.h"
#include "types.h"
#include "helper.h"

esp_err_t _abaWifimanEventRegister(AbaWifiman *app);

esp_err_t _abaWifimanEventUnregister(AbaWifiman *app);

void _abaWifimanEventHandler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data);

void _abaWifimanFallback(AbaWifiman *app);

#endif