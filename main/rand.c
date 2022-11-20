#include <stdio.h>
#include <unistd.h>

// espressif
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "sdkconfig.h"

// canokey
#include "rand.h"

uint32_t random32()
{
    return esp_random();
}

void random_buffer(uint8_t *buf, size_t len)
{
    esp_fill_random(buf, len);
}