#ifndef __WIFI_H__
#define __WIFI_H__

#include "config.h"

#include "nvs_flash.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"
#include "esp_event.h"

#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG_WIFI = "wifi";
static int s_retry_num = 0;
// WPA-WPA2-Personal
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""

static EventGroupHandle_t s_wifi_event_group;

extern QueueHandle_t msg_queue;
extern uint8_t wifiFlag;

//For wifi
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_WIFI,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
//Init wifi
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
    }
}

void wifi(){
        //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}


void tcp_client(void *pvParameter){
    while(1){
        if (wifiFlag){
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            ESP_LOGI(TAG_WIFI, "tcp client Idle");
            continue;
        }
        // char *rx_buffer = (char *)pvPortMalloc(PACKET_SIZE * sizeof(char));
        //char rx_buffer[PACKET_SIZE];
        char host_ip[] = HOST_IP_ADDR;
        int addr_family = 0;
        int ip_protocol = 0;
        uint32_t byteCount = 0;
        int len = 1;

        while (1) {
            if(len == 0){
                ESP_LOGI(TAG_WIFI, "Done Receiving");
                wifiFlag = 1;
                break;
                // TaskDelay(5000 / portTICK_PERIOD_MS);
            }
            struct sockaddr_in dest_addr;
            inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(PORT);
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;

            int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
            if (sock < 0) {
                ESP_LOGE(TAG_WIFI, "Unable to create socket: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG_WIFI, "Socket created, connecting to %s:%d", host_ip, PORT);

            int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err != 0) {
                ESP_LOGE(TAG_WIFI, "Socket unable to connect: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG_WIFI, "Successfully connected");
            while (1) {
                // int err = send(sock, payload, strlen(payload), 0);
                // if (err < 0) {
                //     ESP_LOGE(TAG_WIFI, "Error occurred during sending: errno %d", errno);
                //     break;
                // }
                // ESP_LOGE(TAG_WIFI, "Payload Sent to Server");
                char *rx_buffer = (char *)pvPortMalloc(PACKET_SIZE * sizeof(char));
                len = recv(sock, rx_buffer, PACKET_SIZE * sizeof(char), 0);
                // Error occurred during receiving
                if (len < 0) {
                    ESP_LOGE(TAG_WIFI, "recv failed: errno %d", errno);
                    break;
                }
                else if (len == 0) {
                    ESP_LOGE(TAG_WIFI, "recv 0 bytes");
                    break;
                }
                // Data received
                else {
                // rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                    // ESP_LOGI(TAG_WIFI, "Received %d bytes from %s:", len, host_ip);
                    byteCount+=len;
                    ESP_LOGI(TAG_WIFI, "Received %d bytes %.4f, %x%x", len,(float)byteCount/(1872*1404/2),rx_buffer[0],rx_buffer[1]);
                //ESP_LOGI(TAG_WIFI, "MESSAGE IS %x%x", rx_buffer[0],rx_buffer[1]);
                    struct message *m = (struct message *)pvPortMalloc(sizeof(struct message));
                //  m->size = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                    m->size = (uint16_t)len;
                    m->packet = (uint8_t *)rx_buffer;
                //  ESP_LOGI(TAG_WIFI, "Put in queue Received %d bytes from %c:", (int)m->size, (char)m->packet[0]);
                    while(uxQueueSpacesAvailable(msg_queue)==0){
                        vTaskDelay(200 / portTICK_PERIOD_MS);
                    }
                    if (xQueueSendToBack(msg_queue, (void *)&m, 10000 / portTICK_PERIOD_MS) != pdTRUE) {
                        // Serial.println("Queue full");
                        ESP_LOGE(TAG_WIFI, "Queue full");
                    }
                    ESP_LOGI(TAG_WIFI, "Wifi Spaces Left:%d", uxQueueSpacesAvailable(msg_queue));
                }
            }
            if (len == 0){
                continue;
            }
            if (sock != -1) {
                ESP_LOGE(TAG_WIFI, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }

        }
    }
}


#endif
