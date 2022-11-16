# espressif-SLAAC
Basic WiFi code template setting up a wifi station. Fetches addresses via DHCPv4 and SLAAC

## Caveats
 - Don't forget to set `CONFIG_LWIP_IPV6_AUTOCONFIG` in `pfatformio.ini`, otherwise ESP32 won't do SLAAC.
 - ESP32 generates slaac addresses with EUI-64
