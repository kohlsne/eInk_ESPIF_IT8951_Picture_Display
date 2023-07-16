#include "config.h"
#include "display.h"
#include "wifi.h"
#include "sleep.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"




#include "nvs_flash.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"
#include "esp_event.h"

#include <string.h>
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "sdkconfig.h"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>



static const char *TAG_MAIN = "Main";

// Globals
QueueHandle_t msg_queue;
// SemaphoreHandle_t displaySemaphore;     // Waits for parameter to be read
uint8_t wifiFlag;
uint8_t displayFlag;

spi_device_handle_t spi;


//This is test function to put data into the queue
//Use this task instead of tcp_client to fill the queue
void createMessages(void *pvParameter){
    uint8_t shadeit = 0;
    uint8_t shade = 0;
    for(uint32_t packetNum =0; packetNum < (IT8951_PANEL_WIDTH * IT8951_PANEL_HEIGHT)/2/1400; packetNum++){
        shade = shadeit | (shadeit << 4);
        shadeit++;
        struct message *m = (struct message *)pvPortMalloc(sizeof(struct message));
            m->size = 1400;
            m->packet = (uint8_t *)pvPortMalloc(PACKET_SIZE * sizeof(uint8_t));;
            memset(m->packet, shade, sizeof(uint8_t) * m->size);       //Zero out the transaction
            if (xQueueSendToBack(msg_queue, (void *)&m, 500 / portTICK_PERIOD_MS) != pdTRUE) {
                ESP_LOGE(TAG_MAIN, "Queue full not last packet");
            }
        if (shadeit == 0){
            shadeit=0xF;
        }
        else{
            shadeit=0;
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    //Last packet
    struct message *m = (struct message *)pvPortMalloc(sizeof(struct message));
    m->size = (IT8951_PANEL_WIDTH * IT8951_PANEL_HEIGHT/2)%1400;
    m->packet = (uint8_t *)pvPortMalloc(PACKET_SIZE * sizeof(uint8_t));;
    memset(m->packet, shade, sizeof(uint8_t) * m->size);       //Zero out the transaction
    if (xQueueSendToBack(msg_queue, (void *)&m, 500 / portTICK_PERIOD_MS) != pdTRUE) {
        // Serial.println("Queue full");
        ESP_LOGE(TAG_MAIN, "Queue full last packet");
    }
    ESP_LOGE(TAG_MAIN, "Done Creating Messages");

    while(1){
         vTaskDelay(5000 / portTICK_PERIOD_MS);
    }


}

void pinSetup(){
    gpio_reset_pin(DOTSTAR_PWR);
    gpio_reset_pin(DOTSTAR_DATA);
    gpio_reset_pin(DOTSTAR_CLK);
    gpio_set_direction(DOTSTAR_PWR, GPIO_MODE_OUTPUT);
    gpio_set_direction(DOTSTAR_DATA, GPIO_MODE_INPUT);
    gpio_set_direction(DOTSTAR_CLK, GPIO_MODE_INPUT);
    gpio_set_level(DOTSTAR_PWR, 1);


    gpio_reset_pin(ENABLE_5V);
    gpio_reset_pin(SPI_HRDY);
    gpio_reset_pin(SPI_RST);
    gpio_reset_pin(SPI_CS);
    gpio_set_direction(ENABLE_5V, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_HRDY, GPIO_MODE_INPUT);
    gpio_set_direction(SPI_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE_5V, 1);
    gpio_set_level(SPI_RST, 1);
    gpio_set_level(SPI_CS, 1);
}


/* SPI Config */
void spi_init(void) {
    esp_err_t ret;
    //spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=SPI_MISO,
        .mosi_io_num=SPI_MOSI,
        .sclk_io_num=SPI_CLK,
        .quadwp_io_num=-1,//GPIO pin for WP (Write Protect) signal, or -1 if not used.
        .quadhd_io_num=-1,//GPIO pin for HD (Hold) signal, or -1 if not used.
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz=SPI_MASTER_FREQ_20M,     //Clock out at 20 MHz
        .mode=0,                                //SPI mode 0 SPI mode, representing a pair of (CPOL, CPHA) configuration:
        .spics_io_num=SPI_CS,                   //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .command_bits=0,
        .address_bits=0,
        .dummy_bits=0,
    };
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void manageTasks(void *pvParameter){
    esp_err_t ret;
    wifiFlag = 0;
    displayFlag = 0;

    example_deep_sleep_register_rtc_timer_wakeup();
    example_deep_sleep_register_touch_wakeup();

    xTaskCreate(&display_task, "display_task", 8192,NULL,5,NULL );
    xTaskCreate(&tcp_client, "tcp_client", 2048,NULL,5,NULL );

    while(1){
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG_MAIN, "Manage Tasks Idle");
        if (wifiFlag && displayFlag){
            ESP_LOGI(TAG_MAIN, "Starting Shutdown");
            ret=spi_bus_remove_device(spi);
            ESP_ERROR_CHECK(ret);
            esp_wifi_stop();
            ESP_LOGI(TAG_MAIN, "Deep sleep mode start");
            esp_deep_sleep_start();
            ESP_LOGI(TAG_MAIN, "Awake");
            esp_wifi_restore();
            spi_init();
            example_deep_sleep_register_rtc_timer_wakeup();
            example_deep_sleep_register_touch_wakeup();
            wifiFlag = 0;
            displayFlag = 0;
        }
    }
}

void app_main(void){
   pinSetup();
   wifi();
   spi_init();
   esp_sleep_enable_touchpad_wakeup();

    msg_queue = xQueueCreate(QUEUE_LENGTH, sizeof(struct message *));
    if ( msg_queue == 0){
         ESP_LOGI(TAG_MAIN, "Queue Could not be created");
    }
    xTaskCreate(&manageTasks, "manage_task", 2048,NULL,4,NULL );
  // xTaskCreate(&toggle5v, "toggle5v", 2048,NULL,4,NULL );
        //xTaskCreate(&createMessages, "createMessages", 2048,NULL,5,NULL );
}



