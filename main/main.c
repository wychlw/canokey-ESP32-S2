#include <stdio.h>

// espressif
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_private/usb_phy.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "soc/usb_pins.h"
#include "driver/touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// canokey-core
#include "apdu.h"
#include "applets.h"
#include "common.h"
#include "device.h"
#include "usb_device.h"

//
#include "dcd_esp32sx.h"
#include "device-config.h"
#include "littlefs_init.h"

static usb_phy_handle_t phy_hdl;
static const char *TAG = "canokey-esp32";

esp_err_t usb_driver_install()
{
    // Configure USB PHY
    usb_phy_config_t phy_conf = {
        .controller = USB_PHY_CTRL_OTG,
        .otg_mode = USB_OTG_MODE_DEVICE,
        .target = USB_PHY_TARGET_INT,
    };
    ESP_RETURN_ON_ERROR(usb_new_phy(&phy_conf, &phy_hdl), TAG, "Install USB PHY failed");

    dcd_init();
    dcd_int_enable();
    ESP_LOGI(TAG, "TinyUSB Driver installed");
    return ESP_OK;
}

#ifndef HW_VARIANT_NAME
#define HW_VARIANT_NAME "CanoKey ESP32"
#endif

int admin_vendor_hw_variant(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);

    static const char *const hw_variant_str = HW_VARIANT_NAME;
    size_t len = strlen(hw_variant_str);
    memcpy(RDATA, hw_variant_str, len);
    LL = len;
    if (LL > LE)
        LL = LE;

    return 0;
}

int admin_vendor_hw_sn(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);

    static const char *const hw_sn = "20220313";
    memcpy(RDATA, hw_sn, 8);
    LL = 8;
    if (LL > LE)
        LL = LE;

    return 0;
}

int strong_user_presence_test(void)
{
    DBG_MSG("Strong user-presence test is skipped.\n");
    return 0;
}

/* ----------------------DEVICE INIT--------------------- */
void LED_init()
{
    gpio_config_t led_conf = {.intr_type = GPIO_INTR_DISABLE,
                              .mode = GPIO_MODE_OUTPUT,
                              .pin_bit_mask = (1ULL << LED_GPIO_PIN),
                              .pull_down_en = 0,
                              .pull_up_en = 0};
    gpio_config(&led_conf);
}

void Touch_init()
{
    
    ESP_ERROR_CHECK(touch_pad_init());
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_CHANNAL);
}

void periodic_task(void *arg)
{
    uint16_t touch_value = touch_pad_get_status() | (1ULL << TOUCH_CHANNAL);

    set_touch_result(touch_value);

    device_update_led();
}

void periodic_task_init()
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_task, .name = "periodic_task", .arg = NULL};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5 * 1000));
}

void app_main(void)
{
    littlefs_init();

    LED_init();
    Touch_init();
    periodic_task_init();

    set_nfc_state(0);

    usb_device_init();
    usb_driver_install();

    // Step 6: init applets
    DBG_MSG("Init applets\n");
    applets_install();
    init_apdu_buffer();
    DBG_MSG("Done\n");

    while (1)
    {
        device_loop(1);
        device_delay(10);
    }
}
