#include <stdalign.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// espressif
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "sdkconfig.h"

// canokey
#include "common.h"
#include "fs.h"
#include "lfs.h"
#include "lfs_util.h"
#include "memzero.h"

//
#include "device-config.h"
#include "littlefs_init.h"

#define LOOKAHEAD_SIZE 16
#define CACHE_SIZE 128
#define WRITE_SIZE 8
#define READ_SIZE 1

#define FLASH_PAGE_SIZE 0x1000
#define FLASH_SIZE 0x100000

static uint8_t read_buffer[CACHE_SIZE];
static uint8_t prog_buffer[CACHE_SIZE];
static alignas(4) uint8_t lookahead_buffer[LOOKAHEAD_SIZE];
static struct lfs_config config;
extern uint8_t _lfs_begin;

static esp_partition_t *partition;

int littlefs_read(const struct lfs_config *conf, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    size_t abs_off = (block * conf->block_size) + off;
    ESP_ERROR_CHECK(esp_partition_read(partition, abs_off, buffer, size));
    return 0;
}

int littlefs_prog(const struct lfs_config *conf, lfs_block_t block, lfs_off_t off, const void *buff, lfs_size_t size)
{
    size_t abs_off = (block * conf->block_size) + off;
    ESP_ERROR_CHECK(esp_partition_write(partition, abs_off, buff, size));
    return 0;
}

int littlefs_erase(const struct lfs_config *conf, lfs_block_t block)
{
    size_t abs_off = block * conf->block_size;
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, abs_off, conf->block_size));
    return 0;
}

int littlefs_sync(const struct lfs_config *conf)
{
    return 0;
}

void littlefs_init()
{
    memset(&config, 0, sizeof(config));

    config.read = littlefs_read;
    config.read_size = READ_SIZE;
    config.read_buffer = read_buffer;
    config.prog = littlefs_prog;
    config.prog_size = WRITE_SIZE;
    config.prog_buffer = prog_buffer;
    config.erase = littlefs_erase;
    config.sync = littlefs_sync;
    config.block_size = FLASH_PAGE_SIZE;
    config.block_count = FLASH_SIZE / FLASH_PAGE_SIZE;
    config.block_cycles = 100000;
    config.cache_size = CACHE_SIZE;
    config.lookahead_buffer = lookahead_buffer;

    DBG_MSG("Flash %u blocks (%u bytes)\r\n", config.block_count, FLASH_PAGE_SIZE);

    partition = (esp_partition_t *)esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                                            ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, "lfs");
    if (!partition)
    {
        ERR_MSG("Can not find partition lfs!\n");
        return;
    }

    int err;
    for (int retry = 0; retry < 3; retry++)
    {
        err = fs_mount(&config);
        if (!err)
        {
            return;
        }
    }

    DBG_MSG("Formating data area...\r\n");
    fs_format(&config);
    err = fs_mount(&config);
    if (err)
    {
        ERR_MSG("Failed to mount FS after formating\r\n");
        return;
    }
}
