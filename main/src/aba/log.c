#include "aba/log.h"

#define _ABA_LOG_TAKE_MUTEX() xSemaphoreTake(_aba_log.mtx, portMAX_DELAY)
#define _ABA_LOG_GIVE_MUTEX() xSemaphoreGive(_aba_log.mtx)

_AbaLogHandle _aba_log = {
    .mtx = NULL,
    .def_vprintf = NULL,
    .setup = false,

    .save_en = false,
    .dir_path = NULL,
    .bin_path = NULL,
    .log_path_fmt = NULL,
    .file_keep_num = 0,
    .rotation_period = 0,
    .recover_period = 0,
    .f = NULL,
    .is_err = false,
    .check_ts = 0,

    .cb_arg = NULL,
    .cb_func = NULL};

AbaErr abaLogSetup()
{
    if (_aba_log.mtx == NULL)
    {
        _aba_log.mtx = xSemaphoreCreateMutex();
        if (_aba_log.mtx == NULL)
            return ABA_ERR_FAILURE;
    }
    _aba_log.def_vprintf = esp_log_set_vprintf(_abaLogWrapper);
    _aba_log.setup = true;
    return ABA_ERR_NONE;
}

AbaErr abaLogSaveEnable(
    const char *dir_path,
    uint32_t file_keep_num,
    TickType_t rotation_period,
    TickType_t recover_period)
{
    // ignore if setup is not called
    if (!_aba_log.setup)
        return ABA_ERR_FAILURE;

    _ABA_LOG_TAKE_MUTEX();

    // ignore if the save is already enabled
    if (_aba_log.save_en)
    {
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_NONE;
    }

    // ignore if the args are bad
    if (dir_path == NULL ||
        rotation_period < pdMS_TO_TICKS(5 * 1000 * 60) ||
        file_keep_num == 0 ||
        recover_period == 0 ||
        dir_path[strlen(dir_path) - 1] == '/')
    {
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_BAD_ARGS;
    }

    // save the passed arguments
    _aba_log.rotation_period = rotation_period;
    _aba_log.file_keep_num = file_keep_num;
    _aba_log.recover_period = recover_period;

    // save the dir path
    size_t dir_path_len = strlen(dir_path);

    _aba_log.dir_path = (char *)malloc(dir_path_len + 1);
    if (_aba_log.dir_path == NULL)
    {
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_MALLOC_FAILED;
    }

    strcpy(_aba_log.dir_path, dir_path);

    // construct and save the log bin path
    const char *bin_filename = "/.log.bin";
    size_t bin_path_len = dir_path_len + strlen(bin_filename);

    _aba_log.bin_path = (char *)malloc(bin_path_len + 1);
    if (_aba_log.bin_path == NULL)
    {
        _abaLogFreePath();
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_MALLOC_FAILED;
    }

    snprintf(_aba_log.bin_path, bin_path_len + 1, "%s%s", dir_path, bin_filename);

    // construct and save the log filename format path
    const char *log_filename_fmt = "/%010d.log";
    size_t log_path_fmt_len = dir_path_len + strlen(log_filename_fmt);

    _aba_log.log_path_fmt = (char *)malloc(log_path_fmt_len + 1);
    if (_aba_log.log_path_fmt == NULL)
    {
        _abaLogFreePath();
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_MALLOC_FAILED;
    }

    snprintf(_aba_log.log_path_fmt, log_path_fmt_len + 1, "%s%s", dir_path, log_filename_fmt);

    // create the directory if not exist
    int res = mkdir(_aba_log.dir_path, 0755);
    if (res != 0 && errno != EEXIST)
    {
        _abaLogFreePath();
        _ABA_LOG_GIVE_MUTEX();
        return ABA_ERR_FAILURE;
    }

    // check for bin file existence
    struct stat st;
    if (stat(_aba_log.bin_path, &st) != 0)
    {
        FILE *f = fopen(_aba_log.bin_path, "wb");
        if (!f)
        {
            _abaLogFreePath();
            _ABA_LOG_GIVE_MUTEX();
            return ABA_ERR_FAILURE;
        }

        int cnt[2] = {-1, 0};
        if (fwrite(&cnt, sizeof(int), 2, f) != 2)
        {
            fclose(f);
            _abaLogFreePath();
            _ABA_LOG_GIVE_MUTEX();
            return ABA_ERR_FAILURE;
        }

        fflush(f);
        fclose(f);
    }

    // set the save enable flag into true
    _aba_log.save_en = true;
    _ABA_LOG_GIVE_MUTEX();
    return ABA_ERR_NONE;
}

void abaLogSaveDisable()
{
    if (!_aba_log.setup)
        return;

    _ABA_LOG_TAKE_MUTEX();

    if (!_aba_log.save_en)
    {
        _ABA_LOG_GIVE_MUTEX();
        return;
    }

    _abaLogFreePath();
    _aba_log.save_en = false;

    _ABA_LOG_GIVE_MUTEX();
}

void abaLogSetCallback(AbaLogCbFunc cb, void *arg)
{
    _ABA_LOG_TAKE_MUTEX();

    _aba_log.cb_func = cb;
    _aba_log.cb_arg = arg;

    _ABA_LOG_GIVE_MUTEX();
}

int _abaLogWrapper(const char *format, va_list args)
{
    if (!_aba_log.setup)
        return -1;

    _ABA_LOG_TAKE_MUTEX();

    if (_aba_log.save_en)
    {
        va_list args_cp;
        va_copy(args_cp, args);
        _abaLogSaveHandle(format, args_cp);
        va_end(args_cp);
    }

    if (_aba_log.cb_func != NULL)
    {
        va_list args_cp;
        va_copy(args_cp, args);
        _aba_log.cb_func(_aba_log.cb_arg, format, args_cp);
        va_end(args_cp);
    }

    int ret = _aba_log.def_vprintf(format, args);

    _ABA_LOG_GIVE_MUTEX();
    return ret;
}

void _abaLogFreePath()
{
    free(_aba_log.dir_path);
    _aba_log.dir_path = NULL;
    free(_aba_log.bin_path);
    _aba_log.bin_path = NULL;
    free(_aba_log.log_path_fmt);
    _aba_log.log_path_fmt = NULL;
}

void _abaLogSaveHandle(const char *format, va_list args)
{
    TickType_t now_ts = xTaskGetTickCount();

    if (_aba_log.is_err)
    {
        if (now_ts - _aba_log.check_ts < _aba_log.recover_period)
            return;
        _aba_log.check_ts = now_ts;
    }

    if ((_aba_log.is_err) ||
        (now_ts - _aba_log.check_ts >= _aba_log.rotation_period))
    {
        if (_aba_log.f != NULL)
        {
            fclose(_aba_log.f);
            _aba_log.f = NULL;
        }

        int file_idx = _abaLogNextIndex();
        if (file_idx == -1)
            return;

        char filename[_ABA_LOG_PATH_MAX_LEN];
        snprintf(filename, _ABA_LOG_PATH_MAX_LEN, _aba_log.log_path_fmt, file_idx);
        _aba_log.f = fopen(filename, "wb");
        if (_aba_log.f)
        {
            _aba_log.check_ts = now_ts;
            _aba_log.is_err = false;
        }
    }

    if (_aba_log.f != NULL)
    {
        int res = vfprintf(_aba_log.f, format, args);
        if (res <= 0)
        {
            fclose(_aba_log.f);
            _aba_log.f = NULL;
            _aba_log.is_err = true;
            _aba_log.check_ts = now_ts;
        }
    }
}

int _abaLogNextIndex()
{
    FILE *f = fopen(_aba_log.bin_path, "rb+");
    if (!f)
        return -1;

    int cnt[2];
    if (fread(&cnt, sizeof(int), 2, f) != 2)
    {
        fclose(f);
        return -1;
    }
    cnt[1] += 1;

    if (cnt[1] - cnt[0] > _aba_log.file_keep_num)
    {
        char filename[_ABA_LOG_PATH_MAX_LEN];
        snprintf(filename, _ABA_LOG_PATH_MAX_LEN, _aba_log.log_path_fmt, cnt[0]);
        remove(filename);
        cnt[0] += 1;
    }

    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return -1;
    }
    if (fwrite(&cnt, sizeof(int), 2, f) != 2)
    {
        fclose(f);
        return -1;
    }

    fflush(f);
    fclose(f);
    return cnt[1];
}