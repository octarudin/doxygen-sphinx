#include "core/peripheral.h"

const char *CORE_PERIPHERAL_TAG = "core/peripheral";

void corePeripheralConfig(CorePeripheral *p)
{
    esp_err_t err;
    AbaErr herr;

#if PCG_NVS_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

#if PCG_NVS_RESET == 1
    err = abaPeripheralNVSReset();
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to reset NVS: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogWarn(
        CORE_PERIPHERAL_TAG,
        "NVS reset");
#endif
    err = abaPeripheralNVSSetup(
        &p->nvs,
        PCG_NVS_NAMESPACE,
        PCG_NVS_RESET_ON_FAIL);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup NVS: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "NVS setup success");
#endif

#if PCG_LFS_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

    err = abaPeripheralLFSSetup(
        PCG_LFS_BASE_PATH,
        PCG_LFS_PARTITION_LABEL,
        PCG_LFS_FORMAT_ON_FAIL);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup LFS: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "LFS setup success");
#endif

#if PCG_SD_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

    err = abaPeripheralSDSetup(
        &p->sd_card,
        PCG_SD_SPI_HOST,
        PCG_SD_CS_PIN,
        PCG_SD_BASE_PATH,
        PCG_SD_MAX_FILES,
        PCG_SD_ALLOC_UNIT_SIZE,
        PCG_SD_FORMAT_ON_FAIL);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup SD: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "SD setup success");

#if PCG_SD_LOG_SAVE_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

    herr = abaLogSaveEnable(
        PCG_SD_LOG_SAVE_DIR_PATH,
        PCG_SD_LOG_SAVE_FILE_KEEP_NUM,
        PCG_SD_LOG_SAVE_FILE_ROTATION_PERIOD,
        PCG_SD_LOG_SAVE_RECOVER_PERIOD);
    if (herr != ABA_ERR_NONE)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to enable SD log save: %s",
            abaErrToStr(herr));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "SD log save enable success");
#endif
#endif

#if PCG_WIFI_ENABLE == 1 || PCG_ETH_ENABLE == 1 || PCG_SIM_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

    err = abaPeripheralNetifSetup();
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup netif: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "netif setup success");
#endif

#if PCG_WIFI_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif
    err = abaPeripheralWiFiSetup(
        &p->wifi_ap_netif,
        &p->wifi_sta_netif);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup WiFi: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "WiFi setup success");
#endif

#if PCG_ETH_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

#if PCG_ETH_TYPE == ABA_PERIPHERAL_ETH_TYPE_W5500
    err = abaPeripheralEthW5500Setup(
        &p->eth_netif,
        PCG_ETH_SPI_HOST,
        PCG_ETH_SPI_CLOCK_SPEED,
        PCG_ETH_CS_PIN,
        PCG_ETH_INT_PIN,
        PCG_ETH_RST_PIN);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup W5500 eth: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "W5500 eth setup success");
#endif
#endif

#if PCG_SIM_ENABLE == 1
#ifndef PERIPHERAL_USED
#define PERIPHERAL_USED
#endif

    err = abaPeripheralSIMSetup(
        &p->sim_netif,
        &p->sim_dce,
        PCG_SIM_MODEM_DEVICE_TYPE,
        PCG_SIM_APN,
        PCG_SIM_PIN,
        PCG_SIM_UART_PORT,
        PCG_SIM_PWRKEY_PIN,
        PCG_SIM_TX_PIN,
        PCG_SIM_RX_PIN,
        PCG_SIM_BAUDRATE);
    if (err != ESP_OK)
    {
        abaLogError(
            CORE_PERIPHERAL_TAG,
            "failed to setup SIM: %s",
            esp_err_to_name(err));
        goto restart_device;
    }
    abaLogInfo(
        CORE_PERIPHERAL_TAG,
        "SIM setup success");
#endif

    return;

#ifdef PERIPHERAL_USED
restart_device:
    abaLogWarn(
        CORE_PERIPHERAL_TAG,
        "restarting device in 3s...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
#endif
}
