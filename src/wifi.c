#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WiFi-Handler";
static EventGroupHandle_t wifi_event_group;

//Setup NVS
static void initNVS(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG,"Erasing NVS-flash due to failure in initialisation");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
}


void show_interface(esp_netif_t *intf){
    size_t macbuffersize = 64;
    uint8_t* macbuffer = (uint8_t*)malloc(macbuffersize);
    memset(macbuffer, 0, macbuffersize);
    esp_netif_get_mac(intf, macbuffer);
    printf("Interface %s\n", esp_netif_get_desc(intf));
    printf("\tMAC: %02x:%02x:%02x:%02x:%02x:%02x\n", macbuffer[0], macbuffer[1], macbuffer[2], macbuffer[3], macbuffer[4], macbuffer[5]);
    free(macbuffer);
    /*
        IPv4
    */
    esp_netif_ip_info_t info;
    esp_netif_get_ip_info(intf, &info);
    // str buffer
    char* ipstr = (char*)malloc(INET_ADDRSTRLEN);
    strcpy(ipstr, "");
    // parse IP
    inet_ntop(AF_INET, &(info.ip.addr), ipstr, INET_ADDRSTRLEN);
    // cidr
    uint8_t cidr = 0;
    for(uint8_t i = 0; i < 8*sizeof(info.netmask.addr); i++)
        cidr += info.netmask.addr & (1 << i) ? 1 : 0;
    printf("\tIPv4: %s/%d\n", ipstr, cidr);
    free(ipstr);
    /*
        IPv6
    */
    char* ip6str = (char*)malloc(INET6_ADDRSTRLEN);
    strcpy(ip6str, "unassigned");
    esp_ip6_addr_t ll;
    //Link Local
    if(ESP_OK == esp_netif_get_ip6_linklocal(intf, &ll))
        inet_ntop(AF_INET6, &(ll.addr), ip6str, INET6_ADDRSTRLEN);
        
    printf("\tIPv6 Link-Local: %s\n", ip6str);
    free(ip6str);

    // Global unicast addresses
    printf("\tGlobal unicast address(es):\n");
    esp_ip6_addr_t ip6[LWIP_IPV6_NUM_ADDRESSES];
    for(uint8_t i = 0; i < esp_netif_get_all_ip6(intf, ip6); i++){
        printf("\t\t" IPV6STR "\n", IPV62STR(ip6[i]));
    }  
}

static void l2event_handler(void *intf, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if(event_base != WIFI_EVENT)
        return;

    switch (event_id){
    case WIFI_EVENT_STA_START:
            esp_wifi_connect();
        break;

    case WIFI_EVENT_STA_CONNECTED:{
            // Station interface got connected, get linklocal address
            if(ESP_OK != esp_netif_create_ip6_linklocal(intf)){
                ESP_LOGW(TAG, "Failed to get an IPv6 Link-Local address!");
            }
        }
        break;
    
    case WIFI_EVENT_STA_DISCONNECTED:{
            //on disconnect, connect again
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
        break;
    
    default:
        break;
    }
}

static void dhcp_and_slaac(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if(event_base != IP_EVENT)
        return;

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    
    case IP_EVENT_GOT_IP6:{
            // Interface got an IPv6 address -> display it

            // Determine which type of address it was (comes in event data)
            ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;

            ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR, esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip));
        }
        break;

    default:
        break;
    }
}

esp_netif_t* wifiInit(){
    initNVS();
    // Start TCP/IP
    esp_netif_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create netif
    esp_netif_t* netif = NULL;
    netif = esp_netif_create_default_wifi_sta();

    assert(netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &l2event_handler, netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &dhcp_and_slaac, NULL));

    // Error handling
    #ifndef SSID
        #error Define SSID to connect to!
    #endif
    #ifndef PSK
        #error Define PSK for WPA2!
    #endif

    wifi_config_t wifiConfig = {
        .sta = {
            .ssid = SSID,
            .password = PSK,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };

    ESP_LOGI(TAG, "Connecting via SSID:\t%s", wifiConfig.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
    esp_wifi_connect();

    ESP_LOGI(TAG,"Wifi statred successfully!");

    //vTaskDelay(3000 / portTICK_RATE_MS);
    //show_interface(netif);
    return netif;
}

esp_err_t wifiDeInit(){
    return ESP_OK;
}