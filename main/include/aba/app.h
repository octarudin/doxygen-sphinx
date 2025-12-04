#ifndef __ABA_APP_H
#define __ABA_APP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "aba/error.h"

#define _ABA_APP_START_TIMEOUT pdMS_TO_TICKS(1000)   /**< @brief Max time to wait for task to confirm it's running. */
#define _ABA_APP_TOLERANCE_DELAY pdMS_TO_TICKS(1000) /**< @brief Generic tolerance for state change confirmations. */
#define _ABA_APP_EVENT_RUNNING_BIT (1 << 0)          /**< @brief Bit set by task when it's alive (cleared by start). */
#define _ABA_APP_EVENT_STOPPED_BIT (1 << 1)          /**< @brief Bit set by task just before it self-deletes. */
#define _ABA_APP_EVENT_PAUSED_BIT (1 << 2)           /**< @brief Bit set by task when it enters the paused state. */
#define _ABA_APP_EVENT_RESUMED_BIT (1 << 3)          /**< @brief Bit set by task when it exits the paused state. */
#define _ABA_APP_EVENT_RESUME_CMD_BIT (1 << 4)       /**< @brief Bit set by abaAppResume() to command the task to resume. */
#define _ABA_APP_EXIT_QUEUE_LEN 2                    /**< @brief Max items in the global app exit queue. */
#define _ABA_APP_PASS_CODE 0xFFFFFFFF                /**< @brief Pass return code for the app. */

typedef enum
{
    ABA_APP_PRIORITY_IDLE_0,     /**< Priority 0 (tskIDLE_PRIORITY) */
    ABA_APP_PRIORITY_IDLE_1,     /**< Priority 1 */
    ABA_APP_PRIORITY_IDLE_2,     /**< Priority 2 */
    ABA_APP_PRIORITY_IDLE_3,     /**< Priority 3 */
    ABA_APP_PRIORITY_IDLE_4,     /**< Priority 4 */
    ABA_APP_PRIORITY_LOW_0,      /**< Priority 5 */
    ABA_APP_PRIORITY_LOW_1,      /**< Priority 6 */
    ABA_APP_PRIORITY_LOW_2,      /**< Priority 7 */
    ABA_APP_PRIORITY_LOW_3,      /**< Priority 8 */
    ABA_APP_PRIORITY_LOW_4,      /**< Priority 9 */
    ABA_APP_PRIORITY_MODERATE_0, /**< Priority 10 */
    ABA_APP_PRIORITY_MODERATE_1, /**< Priority 11 */
    ABA_APP_PRIORITY_MODERATE_2, /**< Priority 12 */
    ABA_APP_PRIORITY_MODERATE_3, /**< Priority 13 */
    ABA_APP_PRIORITY_MODERATE_4, /**< Priority 14 */
    ABA_APP_PRIORITY_HIGH_0,     /**< Priority 15 */
    ABA_APP_PRIORITY_HIGH_1,     /**< Priority 16 */
    ABA_APP_PRIORITY_HIGH_2,     /**< Priority 17 */
    ABA_APP_PRIORITY_HIGH_3,     /**< Priority 18 */
    ABA_APP_PRIORITY_HIGH_4,     /**< Priority 19 */
    ABA_APP_PRIORITY_REALTIME_0, /**< Priority 20 */
    ABA_APP_PRIORITY_REALTIME_1, /**< Priority 21 */
    ABA_APP_PRIORITY_REALTIME_2, /**< Priority 22 */
    ABA_APP_PRIORITY_REALTIME_3, /**< Priority 23 */
    ABA_APP_PRIORITY_REALTIME_4  /**< Priority 24 (configMAX_PRIORITIES - 1) */
} AbaAppPriority;

typedef enum
{
    ABA_APP_EXIT_ACTION_FREE,
    ABA_APP_EXIT_ACTION_RESTART,
    ABA_APP_EXIT_ACTION_RESTART_UNLESS_STOPPED,
    ABA_APP_EXIT_ACTION_RESTART_DEVICE
} AbaAppExitAction;

typedef int (*AbaAppCallback)(void *arg);

typedef void (*AbaAppVoidCallback)(void *arg);

extern QueueHandle_t _aba_app_exit_q;

typedef struct
{
    char *name;                   /**< @brief The name for the FreeRTOS task.
                                   * @note A deep copy of this string is made,
                                   * so the original can be a stack variable. */
    AbaAppPriority priority;      /**< @brief Task priority, see AbaAppPriority. */
    size_t stack_size;            /**< @brief Stack size for the task, in bytes. */
    TickType_t sleep_tick;        /**< @brief Ticks to delay in the on_loop callback (vTaskDelay). */
    TickType_t post_setup_delay;  /**< @brief Ticks to delay between the setup and the loop.*/
    TickType_t pre_exit_delay;    /**< @brief Ticks to delay between the loop and exit action.*/
    void *param;                  /**< @brief A user-defined parameter that will be passed
                                   * to all callbacks. */
    AbaAppExitAction exit_action; /**< @brief Action to take after this task stops.
                                   * Processed by the central app manager. */
} AbaAppConfig;

typedef struct
{
    AbaAppCallback on_setup;
    AbaAppCallback on_loop;
    AbaAppVoidCallback on_paused;
    AbaAppVoidCallback on_resumed;
    AbaAppVoidCallback on_stopped;
} AbaAppCallbackGroup;

typedef struct
{
    TaskHandle_t _th;        /**< @internal Task handle for the application. */
    EventGroupHandle_t _ev;  /**< @internal Event group for state synchronization. */
    AbaAppConfig _cfg;       /**< @internal A copy of the user's configuration. */
    AbaAppCallbackGroup _cb; /**< @internal A copy of the user's callbacks. */
    volatile int _exit_code; /**< @internal Stores the exit code returned by a callback. */
    volatile bool _ok;       /**< @internal Internal flag to control the main loop (true = run). */
    volatile bool _sus;      /**< @internal Internal flag to control pause state (true = pause). */

} AbaAppHandle;

AbaAppHandle *abaAppNew(AbaAppConfig *cfg);

AbaErr abaAppSetCallbackGroup(AbaAppHandle *h, AbaAppCallbackGroup *cb);

AbaErr abaAppStart(AbaAppHandle *h);

AbaErr abaAppStop(AbaAppHandle *h);

AbaErr abaAppPause(AbaAppHandle *h);

AbaErr abaAppResume(AbaAppHandle *h);

int abaAppExitCode(AbaAppHandle *h);

inline int abaAppExit(int code) { return ((code == _ABA_APP_PASS_CODE) ? -1 : code); }

inline int abaAppPass() { return _ABA_APP_PASS_CODE; }

void _abaAppTaskWrapper(void *pvParameter);

bool _abaAppInit();

AbaErr _abaAppDelete(AbaAppHandle *h);

#endif