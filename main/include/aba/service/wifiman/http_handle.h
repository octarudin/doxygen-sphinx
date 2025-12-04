#ifndef __WIFIMAN_HTTP_HANDLE_H
#define __WIFIMAN_HTTP_HANDLE_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "aba/log.h"
#include "types.h"
#include "html.h"

extern httpd_uri_t _aba_wifiman_get_root;
extern httpd_uri_t _aba_wifiman_get_api_scanned;
extern httpd_uri_t _aba_wifiman_get_api_status;
extern httpd_uri_t _aba_wifiman_get_api_connsts;
extern httpd_uri_t _aba_wifiman_post_api_scan;
extern httpd_uri_t _aba_wifiman_post_api_connect;
extern httpd_uri_t _aba_wifiman_post_api_commit;

esp_err_t _abaWifimanHTTPHandleRoute(httpd_handle_t server, void *ctx);

void _abaWifimanHTTPHandleUnroute(httpd_handle_t server);

esp_err_t _abaWifimanGetRoot(httpd_req_t *req);

esp_err_t _abaWifimanGetAPIScanned(httpd_req_t *req);

esp_err_t _abaWifimanGetAPIStatus(httpd_req_t *req);

esp_err_t _abaWifimanGetAPIConnsts(httpd_req_t *req);

esp_err_t _abaWifimanPostAPIScan(httpd_req_t *req);

esp_err_t _abaWifimanPostAPIConnect(httpd_req_t *req);

esp_err_t _abaWifimanPostAPICommit(httpd_req_t *req);

esp_err_t _abaWifimanSendErrorResponse(httpd_req_t *req, uint16_t code, const char *msg);

#endif