#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include <stdio.h>
#include <string.h>

//My Password Data
#include "/home/david/Logins/WLAN.h"
#ifndef SSID
    #define SSID WAP_HOME_SSID
#endif
#ifndef PSK
    #define PSK WAP_HOME_PASSWORD
#endif

#define WIFI_MAXIMUM_RETRY 12

/*
    @brief init wifi and do DHCPv4 and SLAAC. Requires macros "SSID" and "PSK"
    @returns NULL on failure. NETIF on success
*/
esp_netif_t* wifiInit();
/*
    @brief displays interface details to stdout
    @param intf interface
*/
void show_interface(esp_netif_t *intf);
esp_err_t wifiDeInit();