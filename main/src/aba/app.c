#include "aba/app.h"

QueueHandle_t _aba_app_exit_q = NULL;

AbaAppHandle *abaAppNew(AbaAppConfig *cfg)
{
    if (_aba_app_exit_q == NULL || cfg == NULL)
        return NULL;

    AbaAppHandle *new = (AbaAppHandle *)malloc(sizeof(AbaAppHandle));
    if (new == NULL)
        return NULL;

    new->_ev = xEventGroupCreate();
    if (new->_ev == NULL)
    {
        free(new);
        return NULL;
    }

    new->_th = NULL;

    new->_cfg.name = (char *)malloc(strlen(cfg->name) + 1);
    if (new->_cfg.name == NULL)
    {
        vEventGroupDelete(new->_ev);
        free(new);
        return NULL;
    }
    strcpy(new->_cfg.name, cfg->name);

    new->_cfg.priority = cfg->priority;
    new->_cfg.stack_size = cfg->stack_size;
    new->_cfg.sleep_tick = cfg->sleep_tick;
    new->_cfg.post_setup_delay = cfg->post_setup_delay;
    new->_cfg.pre_exit_delay = cfg->pre_exit_delay;
    new->_cfg.param = cfg->param;
    new->_cfg.exit_action = cfg->exit_action;

    new->_cb.on_setup = NULL;
    new->_cb.on_loop = NULL;
    new->_cb.on_paused = NULL;
    new->_cb.on_resumed = NULL;
    new->_cb.on_stopped = NULL;

    new->_exit_code = _ABA_APP_PASS_CODE;
    new->_ok = false;
    new->_sus = false;

    return new;
}

AbaErr abaAppSetCallbackGroup(AbaAppHandle *h, AbaAppCallbackGroup *cb)
{
    if (h == NULL || cb == NULL)
        return ABA_ERR_BAD_ARGS;

    h->_cb.on_setup = cb->on_setup;
    h->_cb.on_loop = cb->on_loop;
    h->_cb.on_paused = cb->on_paused;
    h->_cb.on_resumed = cb->on_resumed;
    h->_cb.on_stopped = cb->on_stopped;

    return ABA_ERR_NONE;
}

AbaErr abaAppStart(AbaAppHandle *h)
{
    if (h == NULL)
        return ABA_ERR_BAD_ARGS;

    h->_ok = true;

    if (xTaskCreate(
            _abaAppTaskWrapper,
            h->_cfg.name,
            h->_cfg.stack_size,
            h,
            h->_cfg.priority,
            &h->_th) != pdPASS)
    {
        h->_ok = false;
        return ABA_ERR_FAILURE;
    }

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _ABA_APP_EVENT_RUNNING_BIT,
        pdFALSE,
        pdFALSE,
        _ABA_APP_START_TIMEOUT);
    if ((ev_bits & _ABA_APP_EVENT_RUNNING_BIT) != _ABA_APP_EVENT_RUNNING_BIT)
    {
        vTaskDelete(h->_th);
        h->_th = NULL;
        h->_ok = false;
        return ABA_ERR_TIMEOUT;
    }

    return ABA_ERR_NONE;
}

AbaErr abaAppStop(AbaAppHandle *h)
{
    if (h == NULL)
        return ABA_ERR_BAD_ARGS;

    h->_ok = false;

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _ABA_APP_EVENT_STOPPED_BIT,
        pdTRUE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _ABA_APP_TOLERANCE_DELAY);
    if ((ev_bits & _ABA_APP_EVENT_STOPPED_BIT) != _ABA_APP_EVENT_STOPPED_BIT)
    {
        vTaskDelete(h->_th);
        h->_th = NULL;
        return ABA_ERR_TIMEOUT;
    }

    h->_th = NULL;
    return ABA_ERR_NONE;
}

AbaErr abaAppPause(AbaAppHandle *h)
{
    if (h == NULL)
        return ABA_ERR_BAD_ARGS;

    if ((xEventGroupGetBits(h->_ev) & _ABA_APP_EVENT_PAUSED_BIT) == _ABA_APP_EVENT_PAUSED_BIT)
        return ABA_ERR_NONE;

    h->_sus = true;

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _ABA_APP_EVENT_PAUSED_BIT,
        pdFALSE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _ABA_APP_TOLERANCE_DELAY);
    if ((ev_bits & _ABA_APP_EVENT_PAUSED_BIT) != _ABA_APP_EVENT_PAUSED_BIT)
        return ABA_ERR_TIMEOUT;

    return ABA_ERR_NONE;
}

AbaErr abaAppResume(AbaAppHandle *h)
{
    if (h == NULL)
        return ABA_ERR_BAD_ARGS;

    if ((xEventGroupGetBits(h->_ev) & _ABA_APP_EVENT_PAUSED_BIT) != _ABA_APP_EVENT_PAUSED_BIT)
        return ABA_ERR_NONE;

    xEventGroupSetBits(h->_ev, _ABA_APP_EVENT_RESUME_CMD_BIT);

    EventBits_t ev_bits = xEventGroupWaitBits(
        h->_ev,
        _ABA_APP_EVENT_RESUMED_BIT,
        pdTRUE,
        pdFALSE,
        (h->_cfg.sleep_tick * 2) + _ABA_APP_TOLERANCE_DELAY);
    if ((ev_bits & _ABA_APP_EVENT_RESUMED_BIT) != _ABA_APP_EVENT_RESUMED_BIT)
        return ABA_ERR_TIMEOUT;

    return ABA_ERR_NONE;
}

int abaAppExitCode(AbaAppHandle *h)
{
    if (h == NULL)
        return _ABA_APP_PASS_CODE;

    return h->_exit_code;
}

void _abaAppTaskWrapper(void *pvParameter)
{
    AbaAppHandle *h = (AbaAppHandle *)pvParameter;
    bool sus_notif = false;

    xEventGroupSetBits(h->_ev, _ABA_APP_EVENT_RUNNING_BIT);

    if (h->_cb.on_setup != NULL)
    {
        h->_exit_code = h->_cb.on_setup(h->_cfg.param);
        if (h->_exit_code != _ABA_APP_PASS_CODE)
            h->_ok = false;
    }

    if (h->_ok)
        vTaskDelay(h->_cfg.post_setup_delay);

    while (h->_ok)
    {
        if (h->_sus)
        {
            if (!sus_notif)
            {
                sus_notif = true;
                xEventGroupSetBits(h->_ev, _ABA_APP_EVENT_PAUSED_BIT);

                if (h->_cb.on_paused != NULL)
                    h->_cb.on_paused(h->_cfg.param);
            }

            EventBits_t bits = xEventGroupWaitBits(
                h->_ev,
                _ABA_APP_EVENT_RESUME_CMD_BIT,
                pdTRUE,
                pdFALSE,
                h->_cfg.sleep_tick);
            if ((bits & _ABA_APP_EVENT_RESUME_CMD_BIT) == _ABA_APP_EVENT_RESUME_CMD_BIT)
            {
                h->_sus = false;
                sus_notif = false;
                xEventGroupClearBits(h->_ev, _ABA_APP_EVENT_PAUSED_BIT);
                xEventGroupSetBits(h->_ev, _ABA_APP_EVENT_RESUMED_BIT);

                if (h->_cb.on_resumed != NULL)
                    h->_cb.on_resumed(h->_cfg.param);
            }

            continue;
        }

        if (h->_cb.on_loop != NULL)
        {
            h->_exit_code = h->_cb.on_loop(h->_cfg.param);
            if (h->_exit_code != _ABA_APP_PASS_CODE)
            {
                h->_ok = false;
                break;
            }
        }

        vTaskDelay(h->_cfg.sleep_tick);
    }

    xEventGroupClearBits(h->_ev, _ABA_APP_EVENT_RUNNING_BIT);
    xEventGroupSetBits(h->_ev, _ABA_APP_EVENT_STOPPED_BIT);

    if (h->_cb.on_stopped != NULL)
        h->_cb.on_stopped(h->_cfg.param);

    vTaskDelay(h->_cfg.pre_exit_delay);
    xQueueSend(_aba_app_exit_q, &h, 0);
    vTaskDelete(NULL);
}

bool _abaAppInit()
{
    _aba_app_exit_q = xQueueCreate(_ABA_APP_EXIT_QUEUE_LEN, sizeof(AbaAppHandle *));
    if (_aba_app_exit_q == NULL)
        return false;
    return true;
}

AbaErr _abaAppDelete(AbaAppHandle *h)
{
    if (h == NULL)
        return ABA_ERR_BAD_ARGS;

    free(h->_cfg.name);
    h->_cfg.name = NULL;

    if (h->_ev != NULL)
    {
        vEventGroupDelete(h->_ev);
        h->_ev = NULL;
    }

    free(h);
    return ABA_ERR_NONE;
}