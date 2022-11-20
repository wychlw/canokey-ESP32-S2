#include <stdio.h>
#include <unistd.h>

// espressif
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

// canokey
#include "device.h"

//
#include "device-config.h"

const uint32_t UNTOUCHED_MAX_VAL = 40; /* Suitable for 56K pull-down resistor */
const uint32_t CALI_TIMES = 4;
const uint32_t TOUCH_GAP_TIME = 1500; /* Gap period (in ms) between two consecutive touch events */

void device_delay(int ms)
{
    usleep(ms * 1000);
}

uint32_t device_get_tick()
{
    return esp_timer_get_time();
}

void device_set_timeout(void (*callback)(void), uint16_t timeout)
{
    const esp_timer_create_args_t event_timer_args = {.callback = callback, .arg = NULL, .name = "event"};
    esp_timer_handle_t event_timer;

    ESP_ERROR_CHECK(esp_timer_create(&event_timer_args, &event_timer));

    ESP_ERROR_CHECK(esp_timer_start_once(event_timer, timeout * 1000));
}

int device_spinlock_lock(volatile uint32_t *lock, uint32_t blocking)
{
    while (*lock)
    {
        if (!blocking)
            return -1;
    }
    *lock = 1;
    return 0;
}

void device_spinlock_unlock(volatile uint32_t *lock)
{
    *lock = 0;
}

int device_atomic_compare_and_swap(volatile uint32_t *var, uint32_t expect, uint32_t update)
{
    if likely((*var == expect))
    {
        *var = update;
        return 0;
    }
    return -1;
}

void led_on()
{
    gpio_set_level(LED_GPIO_PIN,1);
}

void led_off()
{
    gpio_set_level(LED_GPIO_PIN,0);
}