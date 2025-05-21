
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

#define HORIZONTAL_FOV 60.0f
#define HORIZONTAL_FOV_HALF HORIZONTAL_FOV / 2

#define VERTICAL_FOV 30.0f
#define VERTICAL_FOV_HALF VERTICAL_FOV / 2

int HORIZONTAL_PIXELS = 80;
int VERTICAL_PIXELS = 60;
int SAMPLING_FREQ = 50;

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

void capture()
{
    move_servo(0, 90.0f + HORIZONTAL_FOV_HALF);
    move_servo(3, 90.0f - VERTICAL_FOV_HALF);
    sleep_ms(2000);

    float pitch = 75.0f - VERTICAL_FOV_HALF;
    float pitch_step = VERTICAL_FOV / VERTICAL_PIXELS;

    for (int i = 0; i < VERTICAL_PIXELS; i++)
    {
        move_servo(3, pitch);
        move_servo(0, 90.0f + HORIZONTAL_FOV_HALF);
        sleep_ms(250);

        float yaw = 90.0f + HORIZONTAL_FOV_HALF;
        float yaw_step = HORIZONTAL_FOV / HORIZONTAL_PIXELS;

        char pixels_s[(HORIZONTAL_PIXELS + 1) * 4];
        memset(pixels_s, ' ', (HORIZONTAL_PIXELS + 1) * 4);
        sprintf(pixels_s, "%03d ", i);
        pixels_s[(HORIZONTAL_PIXELS + 1) * 4 - 1] = 0;

        for (int j = 0; j < HORIZONTAL_PIXELS; j++)
        {
            uint8_t pixel_i = ~(adc_read() >> 4);
            sprintf(pixels_s + (j + 1) * 4, "%03u ", pixel_i);

            yaw -= yaw_step;
            move_servo(0, yaw);

            sleep_ms(1000 / SAMPLING_FREQ);
        }

        send_http_post(URL_BITMAP, pixels_s);

        pitch += pitch_step;
    }
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

    while (1)
    {
        send_http_post(URL_STATUS, "AB");

        if (recv_http_get(URL_SHOOT, &HORIZONTAL_PIXELS, &VERTICAL_PIXELS, &SAMPLING_FREQ) == 'Y')
        {
            send_http_post(URL_SHOOT, "OK");
            capture();
        }

        sleep_ms(2000);
    }
}

void vLaunch(void)
{
    BaseType_t xStatus;

    xStatus = xTaskCreate(main_task, "BLINK", 1024 * 8, NULL, configMAX_PRIORITIES - 1, NULL);

    if (xStatus != pdPASS)
    {
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
    }

    return 0;
}
