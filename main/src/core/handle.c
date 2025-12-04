#include "core/handle.h"

const char *CORE_HANDLE_TAG = "core/handle";

void coreSetup(Core *core)
{
    AbaErr err = abaLogSetup();
    if (err != ABA_ERR_NONE)
    {
        abaLogError(
            CORE_HANDLE_TAG,
            "failed to setup log: %s",
            abaErrToStr(err));
        coreRestart(3000);
    }
    abaLogInfo(
        CORE_HANDLE_TAG,
        "log setup success");

    if (!_abaAppInit())
    {
        abaLogError(
            CORE_HANDLE_TAG,
            "failed to initiate core app handle");
        coreRestart(3000);
    }

    coreInterfaceConfig(&core->interface);
    corePeripheralConfig(&core->peripheral);
    coreServiceConfig(&core->service, &core->peripheral);
}

void coreLoop(Core *core)
{
    AbaAppHandle *app_handle;
    while (1)
    {
        xQueueReceive(_aba_app_exit_q, &app_handle, portMAX_DELAY);

        if (app_handle->_exit_code == 0)
        {
            abaLogInfo(
                CORE_HANDLE_TAG,
                "%s app exited with code %d",
                app_handle->_cfg.name,
                app_handle->_exit_code);
        }
        else
        {
            abaLogWarn(
                CORE_HANDLE_TAG,
                "%s app exited with code %d",
                app_handle->_cfg.name,
                app_handle->_exit_code);
        }

        switch (app_handle->_cfg.exit_action)
        {
        case ABA_APP_EXIT_ACTION_FREE:
            _coreFreeApp(app_handle);
            break;

        case ABA_APP_EXIT_ACTION_RESTART:
            _coreRestartApp(app_handle);
            break;

        case ABA_APP_EXIT_ACTION_RESTART_UNLESS_STOPPED:
            if (app_handle->_exit_code == _ABA_APP_PASS_CODE)
                _coreFreeApp(app_handle);
            else
                _coreRestartApp(app_handle);
            break;

        case ABA_APP_EXIT_ACTION_RESTART_DEVICE:
            abaLogWarn(
                CORE_HANDLE_TAG,
                "%s app requested a device restart",
                app_handle->_cfg.name);
            _coreFreeApp(app_handle);
            coreRestart(3000);
            break;
        }
    }
}

void coreRestart(uint32_t wait_ms)
{
    abaLogInfo(
        CORE_HANDLE_TAG,
        "restarting device in %u ms...",
        wait_ms);
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    esp_restart();
}

void _coreFreeApp(AbaAppHandle *app_handle)
{
    abaLogInfo(
        CORE_HANDLE_TAG,
        "%s app freed",
        app_handle->_cfg.name);
    _abaAppDelete(app_handle);
}

void _coreRestartApp(AbaAppHandle *app_handle)
{
    abaLogInfo(
        CORE_HANDLE_TAG,
        "restarting %s app...",
        app_handle->_cfg.name);

    app_handle->_exit_code = _ABA_APP_PASS_CODE;
    xEventGroupClearBits(app_handle->_ev, _ABA_APP_EVENT_STOPPED_BIT);

    AbaErr err = abaAppStart(app_handle);
    if (err != ABA_ERR_NONE)
    {
        abaLogError(
            CORE_HANDLE_TAG,
            "failed to restart %s app: %s, restarting device instead...",
            app_handle->_cfg.name,
            abaErrToStr(err));

        _coreFreeApp(app_handle);
        coreRestart(3000);
    }
    abaLogInfo(
        CORE_HANDLE_TAG,
        "%s app restarted successfully",
        app_handle->_cfg.name);
}