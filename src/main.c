
#include <errno.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/platform.h"
#include "hardware/adc.h"

#include "http.h"

void blink_4hz()
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(125));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(125));
}

void blink_1s()
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void init_servo(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f); // 125 MHz / 125 = 1 MHz -> 1 tick = 1 us
    pwm_init(slice, &config, true);
}

void move_servo(uint gpio, float angle)
{
    uint32_t clock_hz = 125000000; // Default system clock
    uint32_t wrap = 20000;         // 20 ms period (50 Hz) in microseconds

    uint slice = pwm_gpio_to_slice_num(gpio);
    uint16_t us = 500 + (2000 / 180) * angle; // linear interpolation 0 - 180 -> 500 - 2500

    // PWM config assumes clkdiv = 125 so counter ticks every 1 us
    pwm_set_wrap(slice, wrap);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), us);
}

void main_task(void *param)
{
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    while (true)
    {
        if (!cyw43_arch_wifi_connect_timeout_ms("tor_relay_25565", "Zuzana68", CYW43_AUTH_WPA2_AES_PSK, 30000))
        {
            break;
        }
    }

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    http_init();

    init_servo(0);
    init_servo(3);

    while (true)
    {
        move_servo(0, 0.0f);
        move_servo(3, 0.0f);
        sleep_ms(2000);

        for (float i = 0.0f; i < 180.0f; i += 180.0f / 40)
        {
            char pixel_s[4];
            uint8_t pixel_i = adc_read() >> 4;
            pixel_i = ~pixel_i;
            sprintf(pixel_s, "%u", pixel_i);
            send_http_post(URL_BITMAP, pixel_s);

            for (float j = 0.0f; j < 180.0f; j += 180.0f / 40)
            {
                move_servo(0, j);
                sleep_ms(20);
            }
            move_servo(3, i);
            move_servo(0, 0.0f);
            sleep_ms(500);
        }
    }
}

void vLaunch(void)
{
    BaseType_t xStatus;

    xStatus = xTaskCreate(main_task, "BLINK", 4096, NULL, configMAX_PRIORITIES - 4, NULL);

    if (xStatus != pdPASS)
    {
        printf("Error: xTaskCreate failed");
        exit(1);
    }
    else
    {
        vTaskStartScheduler();
    }
}

int main(void)
{
    stdio_init_all();

    vLaunch();

    while (1)
    {
        blink_4hz();
    }

    return 0;
}
