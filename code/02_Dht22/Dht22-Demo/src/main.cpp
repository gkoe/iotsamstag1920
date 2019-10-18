
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <Dht22.h>

extern "C"
{
    void app_main(void);
}
const gpio_num_t DHT22_PIN = GPIO_NUM_27;

void app_main()
{
    printf("\n===========\n");
    printf("DHT22 Demo!\n");
    printf("===========\n");

    Dht22 *dht22Ptr = new Dht22();
    dht22Ptr->init(DHT22_PIN);

    while (true)
    {
        float temperature = dht22Ptr->getTemperature();
        float humidity = dht22Ptr->getHumidity();
        printf("Temperature: %.2f °C, Humidity: %.2f %%\n", temperature, humidity);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
