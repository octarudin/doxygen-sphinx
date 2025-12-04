#ifndef __ABA_PERIPHERAL_NVS_H
#define __ABA_PERIPHERAL_NVS_H

#include <stdbool.h>
#include "esp_err.h"
#include "nvs_flash.h"

esp_err_t abaPeripheralNVSReset();

esp_err_t abaPeripheralNVSSetup(
    nvs_handle_t *nvs_handle,
    const char *namespace,
    bool reset_on_fail);

#endif