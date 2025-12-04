#ifndef __CORE_SERVICE_H
#define __CORE_SERVICE_H

#include "esp_err.h"
#include "esp_http_server.h"
#include "aba/error.h"
#include "aba/log.h"
#include "aba/service/service.h"
#include "peripheral.h"

/**
 * @defgroup core_service_cfg Core Service Configuration
 * @brief Macros to enable and configure high-level system services.
 * @{
 */

/** @name HTTP Server Configuration */
///@{
#define SCG_HTTP_SERVER_ENABLE 1 ///< Set to 1 to enable the embedded HTTP server, 0 to disable.
#define SCG_HTTP_SERVER_PORT 80  ///< The TCP port to listen on (default is 80).
///@}

/** @name WiFi Manager Configuration */
///@{
#define SCG_WIFIMAN_ENABLE 1                       ///< Set to 1 to enable the WiFi Manager service (requires NVS and WiFi peripheral).
#define SCG_WIFIMAN_DEFAULT_AP_SSID "HayaOS-AP"    ///< The default SSID for the SoftAP when provisioning.
#define SCG_WIFIMAN_DEFAULT_AP_PASS "Haya12345678" ///< The default password for the SoftAP.

/* Dependency check: Wifiman requires NVS for storage and WiFi for networking */
#if SCG_WIFIMAN_ENABLE == 1 && (PCG_NVS_ENABLE != 1 || PCG_WIFI_ENABLE != 1)
#error "NVS and WiFi peripherals must be enabled to use Wifiman service"
#endif
///@}

/** @name MQTT Client Configuration */
///@{
#define SCG_MQTT_CLIENT_ENABLE 1 ///< Set to 1 to enable the MQTT client service.
///@}

/** @} */ // end of core_service_cfg group

typedef struct
{
#if SCG_HTTP_SERVER_ENABLE == 1
    httpd_handle_t server; /*!< @brief Handle for the HTTP server instance. */
#endif

#if SCG_WIFIMAN_ENABLE == 1
    AbaWifiman *wifiman; /*!< @brief Handle/Pointer to the WiFi Manager service instance. */
#endif

#if SCG_MQTT_CLIENT_ENABLE == 1
#endif
} CoreService;

void coreServiceConfig(CoreService *s, CorePeripheral *p);

#endif