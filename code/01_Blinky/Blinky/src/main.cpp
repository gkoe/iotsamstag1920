
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C"
{
    void app_main(void);
}
const gpio_num_t R = GPIO_NUM_14;
const gpio_num_t G = GPIO_NUM_12;
const gpio_num_t B = GPIO_NUM_13;

void app_main()
{
    printf("\n===========\n");
    printf("Blinky RGB!\n");
    printf("===========\n");

    gpio_pad_select_gpio(R);
    gpio_set_direction(R, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(G);
    gpio_set_direction(G, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(B);
    gpio_set_direction(B, GPIO_MODE_OUTPUT);

    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("RED\n");
        gpio_set_level(R, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(R, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("GREEN\n");
        gpio_set_level(G, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(G, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("BLUE\n");
        gpio_set_level(B, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(B, 0);
    }
}
