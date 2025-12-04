#ifndef __ABA_LOG_H
#define __ABA_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "aba/error.h"

#define abaLogError ESP_LOGE   /**< @brief Log a message at ESP_LOGE level. */
#define abaLogWarn ESP_LOGW    /**< @brief Log a message at ESP_LOGW level. */
#define abaLogInfo ESP_LOGI    /**< @brief Log a message at ESP_LOGI level. */
#define abaLogDebug ESP_LOGD   /**< @brief Log a message at ESP_LOGD level. */
#define abaLogVerbose ESP_LOGV /**< @brief Log a message at ESP_LOGV level. */

#define _ABA_LOG_PATH_MAX_LEN 256

typedef void (*AbaLogCbFunc)(void *arg, const char *format, va_list args);

typedef struct
{
    SemaphoreHandle_t mtx;      /**< @brief Mutex for thread-safe access to the handle. */
    vprintf_like_t def_vprintf; /**< @brief Stores the original ESP-IDF vprintf function. */
    volatile bool setup;        /**< @brief Flag (true) if hyLogSetup() has been called. */
    bool save_en;               /**< @brief Flag (true) if file logging is enabled. */
    char *dir_path;             /**< @brief Path to the log directory (e.g., "/spiffs/logs"). */
    char *bin_path;             /**< @brief Path to the rotation index file (e.g., "/spiffs/logs/.log.bin"). */
    char *log_path_fmt;         /**< @brief Format string for log files (e.g., "/spiffs/logs/%010d.log"). */
    uint32_t file_keep_num;     /**< @brief Max number of log files to keep during rotation. */
    TickType_t rotation_period; /**< @brief Ticks between log file rotations. */
    TickType_t recover_period;  /**< @brief Ticks to wait before retrying a file write after an error. */
    FILE *f;                    /**< @brief The file handle for the *current* open log file. */
    bool is_err;                /**< @brief Flag (true) if a file write error has occurred. */
    TickType_t check_ts;        /**< @brief Timestamp (in ticks) of the last rotation or error check. */
    void *cb_arg;               /**< @brief User-defined argument for the custom callback. */
    AbaLogCbFunc cb_func;       /**< @brief The custom callback function pointer. */
} _AbaLogHandle;

extern _AbaLogHandle _aba_log;

AbaErr abaLogSetup();

AbaErr abaLogSaveEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t file_rotation_period,
    TickType_t recover_period);

void abaLogSaveDisable();

void abaLogSetCallback(AbaLogCbFunc cb, void *arg);

int _abaLogWrapper(const char *format, va_list args);

void _abaLogFreePath();

void _abaLogSaveHandle(const char *format, va_list args);

int _abaLogNextIndex();

#endif