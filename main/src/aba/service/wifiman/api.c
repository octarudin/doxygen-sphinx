#include "aba/service/wifiman/api.h"

const char *WIFIMAN_TAG = "wifiman";

AbaWifiman *abaWifimanNew(
    nvs_handle_t nvs,
    httpd_handle_t server,
    esp_netif_t *ap_netif,
    esp_netif_t *sta_netif,
    const char *ap_default_ssid,
    const char *ap_default_pass)
{
    AbaWifiman *new = (AbaWifiman *)malloc(sizeof(AbaWifiman));
    if (new == NULL)
        return NULL;

    new->nvs = nvs;
    new->server = server;

    new->ap_netif = ap_netif;
    new->sta_netif = sta_netif;
    new->ap_default_ssid = ap_default_ssid;
    new->ap_default_pass = ap_default_pass;

    new->wifi_evh = NULL;
    new->ip_evh = NULL;

    new->scanning = false;
    new->connecting = false;
    new->connected = false;
    new->reason = 0;

    return new;
}

AbaErr abaWifimanStart(AbaWifiman *wm)
{
    esp_err_t err = _abaWifimanNVSInit(wm);
    if (err != ESP_OK)
    {
        abaLogError(
            WIFIMAN_TAG,
            "failed to initiate NVS: %s",
            esp_err_to_name(err));
        return ABA_ERR_FAILURE;
    }

    err = _abaWifimanEventRegister(wm);
    if (err != ESP_OK)
    {
        abaLogError(
            WIFIMAN_TAG,
            "failed to register events: %s",
            esp_err_to_name(err));
        return ABA_ERR_FAILURE;
    }

    err = _abaWifimanWiFiInit(wm);
    if (err != ESP_OK)
    {
        abaLogError(
            WIFIMAN_TAG,
            "failed to initiate WiFi: %s",
            esp_err_to_name(err));

        _abaWifimanEventUnregister(wm);
        return ABA_ERR_FAILURE;
    }

    err = _abaWifimanHTTPHandleRoute(wm->server, (void *)wm);
    if (err != ESP_OK)
    {
        abaLogError(
            WIFIMAN_TAG,
            "failed to route HTTP: %s",
            esp_err_to_name(err));

        _abaWifimanWiFiDeinit();
        _abaWifimanEventUnregister(wm);
        return ABA_ERR_FAILURE;
    }

    abaLogInfo(
        WIFIMAN_TAG,
        "started successfully at /wifiman");
    return ABA_ERR_NONE;
}

AbaErr abaWifimanStop(AbaWifiman *wm)
{
    return ABA_ERR_NONE;
}