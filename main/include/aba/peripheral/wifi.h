#ifndef __ABA_PERIPHERAL_WIFI_H
#define __ABA_PERIPHERAL_WIFI_H

#include <string.h>
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"

esp_err_t abaPeripheralWiFiSetup(
    esp_netif_t **ap_netif,
    esp_netif_t **sta_netif);

#endif