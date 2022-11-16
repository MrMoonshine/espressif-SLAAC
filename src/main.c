#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "wifi.h"

void app_main(void){
    gpio_pad_select_gpio(ONBOARD_LED);
    gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);
    bool ON = 0;

    esp_netif_t* netif = wifiInit();

    while(true)
    {
        show_interface(netif);
        ON = !ON;
        gpio_set_level(ONBOARD_LED, ON);
        vTaskDelay(5000/ portTICK_PERIOD_MS);
    }
}