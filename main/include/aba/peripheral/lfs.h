#ifndef __ABA_PERIPHERAL_LFS_H
#define __ABA_PERIPHERAL_LFS_H

#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_littlefs.h"
#include "aba/log.h"

esp_err_t abaPeripheralLFSSetup(
    const char *base_path,
    const char *partition_label,
    bool format_on_fail);

#endif