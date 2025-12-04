#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "aba/log.h"
#include "aba/app.h"
#include "interface.h"
#include "peripheral.h"
#include "service.h"

typedef struct
{
    CoreInterface interface;   /**< @brief Holds all initialized interface handles (SPI, I2C, etc.). */
    CorePeripheral peripheral; /**< @brief Holds all initialized peripheral handles (NVS, SD, WiFi, etc.). */
    CoreService service;       /**< @brief Holds all initialized service handles (Wifiman, Mqfastt, etc.). */
} Core;

void coreSetup(Core *core);

void coreLoop(Core *core);

void coreRestart(uint32_t wait_ms);

void _coreFreeApp(AbaAppHandle *app_handle);

void _coreRestartApp(AbaAppHandle *app_handle);

#endif