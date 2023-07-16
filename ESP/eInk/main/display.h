#ifndef __DISPLAY_H__ //{__IT8951_I80_DISPLAY_H__
#define __DISPLAY_H__

/*
I changed some of the #define to be little endian. So you may want to check the documentation on what is correct for your system
*/

#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h>
#include "esp_log.h"

struct I80IT8951DevInfo {
    uint16_t usPanelW;
    uint16_t usPanelH;
    uint16_t usImgBufAddrL;
    uint16_t usImgBufAddrH;
    uint16_t usFWVersion[8]; //16 Bytes String
    uint16_t usLUTVersion[8]; //16 Bytes String
};


union dispayInfoUnion{
uint16_t readbuff[20];
struct I80IT8951DevInfo info;
};

union dispayInfoUnion displayInfo;

struct IT8951LdImgInfo{
    uint32_t ulImgBufBaseAddr;//Base address of target image buffer
    uint16_t usEndianType; //little or Big Endian
    uint16_t usPixelFormat; //bpp
    uint16_t usRotate; //Rotate mode
 //   uint32_t ulStartFBAddr; //Start address of source Frame buffer

};

//structure prototype 2
struct IT8951AreaImgInfo{
    uint16_t usX;
    uint16_t usY;
    uint16_t usWidth;
    uint16_t usHeight;

};


//Built in I80 Command Code
#define IT8951_TCON_SYS_RUN      0x0001
#define IT8951_TCON_STANDBY      0x0002
#define IT8951_TCON_SLEEP        0x0003
#define IT8951_TCON_REG_RD       0x1000 //little endian
// #define IT8951_TCON_REG_RD       0x0010
#define IT8951_TCON_REG_WR       0x1100
// #define IT8951_TCON_REG_WR       0x0011
#define IT8951_TCON_MEM_BST_RD_T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR   0x0014
#define IT8951_TCON_MEM_BST_END  0x0015
// #define IT8951_TCON_LD_IMG       0x0020
// #define IT8951_TCON_LD_IMG_AREA  0x0021
// #define IT8951_TCON_LD_IMG_END   0x0022
#define IT8951_TCON_LD_IMG       0x2000//little endian
#define IT8951_TCON_LD_IMG_AREA  0x2100//little endian
#define IT8951_TCON_LD_IMG_END   0x2200//little endian


//I80 User defined command code
// #define USDEF_I80_CMD_DPY_AREA     0x0034
#define USDEF_I80_CMD_DPY_AREA     0x3400//little endian

// #define USDEF_I80_CMD_GET_DEV_INFO 0x0302
// #define IT8951_WRITE_COMMAND_CODE_PREAMBLE  0x6000
// #define IT8951_WRITE_DATA_PREAMBLE  0x0000
// #define IT8951_READ_DATA_PREAMBLE  0x1000
#define VCOM_CMD_CODE 0x3900//little endian
#define USDEF_I80_CMD_GET_DEV_INFO 0x0203//little endian
#define USDEF_I80_CMD_GET_DEV_INFO_SIZE 20
#define IT8951_WRITE_COMMAND_CODE_PREAMBLE  0x0060//little endian
#define IT8951_WRITE_DATA_PREAMBLE  0x0000//little endian
#define IT8951_READ_DATA_PREAMBLE  0x0010//little endian

//Rotate mode
#define IT8951_ROTATE_0     0
#define IT8951_ROTATE_90    1
#define IT8951_ROTATE_180   2
#define IT8951_ROTATE_270   3

//Pixel mode , BPP - Bit per Pixel
#define IT8951_2BPP   0
#define IT8951_3BPP   1
#define IT8951_4BPP   2
#define IT8951_8BPP   3

//Waveform Mode
#define IT8951_MODE_0   0
#define IT8951_MODE_1   1
#define IT8951_MODE_2   2
#define IT8951_MODE_3   3
#define IT8951_MODE_4   4
//Endian Type
#define IT8951_LDIMG_L_ENDIAN   0
#define IT8951_LDIMG_B_ENDIAN   1
//Auto LUT
#define IT8951_DIS_AUTO_LUT   0
#define IT8951_EN_AUTO_LUT    1
//LUT Engine Status
#define IT8951_ALL_LUTE_BUSY 0xFFFF

//-----------------------------------------------------------------------
// IT8951 TCon Registers defines
//-----------------------------------------------------------------------
//Register Base Address
#define DISPLAY_REG_BASE 0x1000               //Register RW access for I80 only
//Base Address of Basic LUT Registers
#define LUT0EWHR  (DISPLAY_REG_BASE + 0x00)   //LUT0 Engine Width Height Reg
#define LUT0XYR   (DISPLAY_REG_BASE + 0x40)   //LUT0 XY Reg
#define LUT0BADDR (DISPLAY_REG_BASE + 0x80)   //LUT0 Base Address Reg
#define LUT0MFN   (DISPLAY_REG_BASE + 0xC0)   //LUT0 Mode and Frame number Reg
#define LUT01AF   (DISPLAY_REG_BASE + 0x114)  //LUT0 and LUT1 Active Flag Reg
//Update Parameter Setting Register
#define UP0SR (DISPLAY_REG_BASE + 0x134)      //Update Parameter0 Setting Reg

#define UP1SR     (DISPLAY_REG_BASE + 0x138)  //Update Parameter1 Setting Reg
#define LUT0ABFRV (DISPLAY_REG_BASE + 0x13C)  //LUT0 Alpha blend and Fill rectangle Value
#define UPBBADDR  (DISPLAY_REG_BASE + 0x17C)  //Update Buffer Base Address
#define LUT0IMXY  (DISPLAY_REG_BASE + 0x180)  //LUT0 Image buffer X/Y offset Reg
// #define LUTAFSR   (DISPLAY_REG_BASE + 0x224)  //LUT Status Reg (status of All LUT Engines)
#define LUTAFSR   0x2412  //LUT Status Reg (status of All LUT Engines)

#define BGVR      (DISPLAY_REG_BASE + 0x250)  //Bitmap (1bpp) image color table
//-------System Registers----------------
#define SYS_REG_BASE 0x0000

//Address of System Registers
// #define I80CPCR (SYS_REG_BASE + 0x04)
#define I80CPCR (0x0400)
//-------Memory Converter Registers----------------
#define MCSR_BASE_ADDR 0x0200
#define MCSR (MCSR_BASE_ADDR  + 0x0000)
// #define LISAR (MCSR_BASE_ADDR + 0x0008)
#define LISAR (0x0802)

extern QueueHandle_t msg_queue;
extern uint8_t displayFlag;
extern spi_device_handle_t spi;
static const char *TAG_SPI = "SPI";


//Host controller function 1---Wait for host data Bus Ready
void waitForDisplayReadyHRDY(){while(!gpio_get_level(SPI_HRDY)){}}


void LCDWriteData(uint16_t data){
    uint32_t cmdCode = (uint32_t)IT8951_WRITE_DATA_PREAMBLE | ((uint32_t)data) << 16;
    //uint8_t rx_data[5];
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8*4;                     //Command is 16 bits
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level(SPI_CS, 1);

}


void LCDWriteNData(uint16_t* data, uint32_t numOfBytes2Write){
    uint16_t cmdCode = IT8951_WRITE_DATA_PREAMBLE;
    //uint8_t rx_data[5];
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8*2;                     //Command is 16 bits
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
        //Should have had no issues.

    for (uint32_t i = 0; i < numOfBytes2Write-1; i++){
        cmdCode = data[i];
        waitForDisplayReadyHRDY();
        ret=spi_device_polling_transmit(spi, &t);  //Transmit!
        assert(ret==ESP_OK);
        if (i%(1872*1404/16) == 0){
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }

    t.flags=0;
    cmdCode = data[numOfBytes2Write-1];
    waitForDisplayReadyHRDY();
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);

    gpio_set_level(SPI_CS, 1);

    ESP_LOGI(TAG_SPI, "Write N value  %x",(int)(data[numOfBytes2Write-1]));
}



void LCDWriteWifiPackets(){
    uint16_t cmdCode = IT8951_WRITE_DATA_PREAMBLE;
    //uint8_t rx_data[5];
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8*2;                     //Command is 16 bits
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);
        //Should have had no issues.
    t.length=8;
    t.rxlength = 0;
    uint8_t twoPixels;
    t.tx_buffer=&twoPixels;
        //Should have had no issues.
    struct message *m;
    uint32_t byteCount = 0;
    while(1){
        while(uxQueueSpacesAvailable(msg_queue)==QUEUE_LENGTH){
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        if(xQueueReceive(msg_queue, &(m), ( TickType_t ) 10 ) ){
            ESP_LOGI(TAG_SPI, "SPI  Spaces Left:%d %x%x", uxQueueSpacesAvailable(msg_queue),m->packet[0],m->packet[1]);
            //ESP_LOGI(TAG2, " take from queue Received %d bytes from %c%c%c%c%c:", (int)m->size, (char)m->packet[0],(char)m->packet[1],(char)m->packet[2],(char)m->packet[3],(char)m->packet[4]);
            if (m->packet == NULL || m == NULL){
                ESP_LOGE(TAG_SPI, "NULL ERROR");
            }
            byteCount += m->size;
            if (byteCount%(1872*1404/64) == 0){
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            if (byteCount >= IT8951_PANEL_WIDTH * IT8951_PANEL_HEIGHT / 2){ //if last data packet
                for (uint32_t i = 0; i < m->size - 1; i++){
                    twoPixels = m->packet[i];
                    // waitForDisplayReadyHRDY();
                    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
                    assert(ret==ESP_OK);
                }
                t.flags=0;
                twoPixels = m->packet[m->size-1];
                // waitForDisplayReadyHRDY();
                ret=spi_device_polling_transmit(spi, &t);  //Transmit!
                assert(ret==ESP_OK);

                ESP_LOGI(TAG_SPI, "Last Byte Written Packet Size: %d",m->size);
                vPortFree(m->packet);
                m->packet = NULL;
                vPortFree(m);
                m = NULL;
                gpio_set_level(SPI_CS, 1);

                break;
            }
            else{ // not the last data packet
                for (uint32_t i = 0; i < m->size; i++){
                    twoPixels = m->packet[i];
                    // waitForDisplayReadyHRDY();
                    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
                    assert(ret==ESP_OK);
                }

                vPortFree(m->packet);
                m->packet = NULL;
                vPortFree(m);
                m = NULL;
            }

        }
        else{
            ESP_LOGE(TAG_SPI, "Queue Empty");
        }
    }
}



void LCDWriteNDataWithValue(uint8_t value, uint32_t numOfBytes2Write){
    uint16_t cmdCode = IT8951_WRITE_DATA_PREAMBLE;
    //uint8_t rx_data[5];
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8*2;                     //Command is 16 bits
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;
    t.rxlength = 0;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);
        //Should have had no issues.
    t.length=8;
    t.rxlength = 0;
    t.tx_buffer=&value;

    for (uint32_t i = 0; i < numOfBytes2Write-1; i++){
    // for (uint32_t i = 0; i < 10; i++){
        value++;
        value = (~(value) << 4) | (value & 0x0f);
     //   waitForDisplayReadyHRDY();
        ret=spi_device_polling_transmit(spi, &t);  //Transmit!
        assert(ret==ESP_OK);
        if (i%(1872*1404/16) == 0){
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }

    t.flags=0;
  //  waitForDisplayReadyHRDY();
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);

    gpio_set_level(SPI_CS, 1);

}


/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
// If spi_transaction_t::rx_buffer is NULL and SPI_TRANS_USE_RXDATA is not set, the Read phase is skipped.
void LCDWriteCmdCode(uint16_t cmd){
    uint32_t cmdCode = (uint32_t)IT8951_WRITE_COMMAND_CODE_PREAMBLE | ((uint32_t)cmd) << 16;
    //uint8_t rx_data[5];
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8*4;                     //Command is 16 bits
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    gpio_set_level(SPI_CS, 1);
}

//If spi_transaction_t::tx_buffer is NULL and SPI_TRANS_USE_TXDATA is not set, the Write phase is skipped.
uint16_t LCDReadData(){
    uint16_t cmdCode = IT8951_READ_DATA_PREAMBLE;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=16;
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;               //The data is the cmd itself
    t.rxlength=16;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    int16_t rx_data;
    t.rx_buffer=&rx_data;               //The data is the cmd itself
    t.tx_buffer=NULL;

    //first word read is dummy
    waitForDisplayReadyHRDY();
    ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    t.flags = 0;
    ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    gpio_set_level(SPI_CS, 1);

    return MY_WORD_SWAP(rx_data);
}


//If spi_transaction_t::tx_buffer is NULL and SPI_TRANS_USE_TXDATA is not set, the Write phase is skipped.
void LCDReadNData(uint16_t *readBuff, uint8_t numOfWords2Read){
    uint16_t cmdCode = IT8951_READ_DATA_PREAMBLE;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=16;
    t.tx_buffer=&cmdCode;               //The data is the cmd itself
    t.rx_buffer=NULL;               //The data is the cmd itself
    t.rxlength=16;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

    waitForDisplayReadyHRDY();
    gpio_set_level(SPI_CS, 0);
    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    int16_t rx_data;
    t.rx_buffer=&rx_data;               //The data is the cmd itself
    t.tx_buffer=NULL;

    //first word read is dummy
    waitForDisplayReadyHRDY();
    ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    for(uint8_t i=0; i<numOfWords2Read-1;i++){
        waitForDisplayReadyHRDY();
        ret = spi_device_polling_transmit(spi, &t);
        assert( ret == ESP_OK );
        readBuff[i]=MY_WORD_SWAP(rx_data);
//        ESP_LOGI(TAG_SPI, "%d Read %x",(int)i,(int)rx_data);
    }
    t.flags = 0;
    ret = spi_device_polling_transmit(spi, &t);
    readBuff[numOfWords2Read-1] = MY_WORD_SWAP(rx_data);
    gpio_set_level(SPI_CS, 1);
  //  ESP_LOGI(TAG_SPI, "%d Read %x,%x",9,(char)t.rx_data[0],(char)t.rx_data[1]);
}
void vcom_get(){
        // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
        spi_device_acquire_bus(spi, portMAX_DELAY);

        LCDWriteCmdCode(VCOM_CMD_CODE);

        LCDWriteData(0x0000);

        uint16_t vcom = LCDReadData();
        // Release bus
        spi_device_release_bus(spi);

        ESP_LOGI(TAG_SPI, "VCOM %x",(int)vcom);

}

void vcom_set(){
    // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
    spi_device_acquire_bus(spi, portMAX_DELAY);

    LCDWriteCmdCode(VCOM_CMD_CODE);
    LCDWriteData(0x0100);
    LCDWriteData(0xe204);

    // Release bus
    spi_device_release_bus(spi);

}


void initDisplay(){
    gpio_set_level(SPI_RST, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(SPI_RST, 1);
    waitForDisplayReadyHRDY();
}


void lcd_get_id(){
    uint8_t count = 0;
    while(1){

        memset(displayInfo.readbuff, 0, sizeof(displayInfo));

        // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
        spi_device_acquire_bus(spi, portMAX_DELAY);

        LCDWriteCmdCode(USDEF_I80_CMD_GET_DEV_INFO);

        LCDReadNData(displayInfo.readbuff, USDEF_I80_CMD_GET_DEV_INFO_SIZE);//read 20 bytes
        // Release bus
        spi_device_release_bus(spi);

        ESP_LOGI(TAG_SPI, "Width and Height: %d,%d",(int)(displayInfo.info.usPanelW),(int)(displayInfo.info.usPanelH));

        if (displayInfo.info.usPanelW == IT8951_PANEL_WIDTH && displayInfo.info.usPanelH == IT8951_PANEL_HEIGHT){break;}
        if ((displayInfo.info.usPanelW != 0 && displayInfo.info.usPanelH != IT8951_PANEL_HEIGHT) || count == 60){
            esp_restart();
        }
        count++;

        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }

}

uint16_t IT8951ReadReg(uint16_t usRegAddr){
    uint16_t read;
    //----------I80 Mode-------------
    //Send Cmd and Register Address
    LCDWriteCmdCode(IT8951_TCON_REG_RD);
    LCDWriteData(usRegAddr);
    //Read data from Host Data bus
    read = LCDReadData();
    //ESP_LOGI(TAG_SPI, "Register  %d",(int)(read));
    return read;
}
void IT8951WaitForDisplayReady(){
    uint16_t readData = 1;
    //Check IT8951 Register LUTAFSR => NonZero ¡V Busy, 0 - Free
    while(readData){
        spi_device_acquire_bus(spi, portMAX_DELAY);
        readData = IT8951ReadReg(LUTAFSR);
        spi_device_release_bus(spi);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void IT8951WriteReg(uint16_t usRegAddr,uint16_t usValue){
 //   spi_device_acquire_bus(spi, portMAX_DELAY);
    //I80 Mode
    //Send Cmd , Register Address and Write Value
    LCDWriteCmdCode(IT8951_TCON_REG_WR);
    LCDWriteData(usRegAddr);
    LCDWriteData(usValue);
  //  spi_device_release_bus(spi);
}

void IT8951SetImgBufBaseAddr(){
    IT8951WriteReg(0x0A02 ,MY_WORD_SWAP(displayInfo.info.usImgBufAddrH));
    IT8951WriteReg(LISAR ,MY_WORD_SWAP(displayInfo.info.usImgBufAddrL));
}

//-----------------------------------------------------------
//Display functions 3 - Application for Display panel Area
//-----------------------------------------------------------
void IT8951DisplayArea(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode){
    spi_device_acquire_bus(spi, portMAX_DELAY);
    //Send I80 Display Command (User defined command of IT8951)
    LCDWriteCmdCode(USDEF_I80_CMD_DPY_AREA); //0x0034
    //Write arguments
    LCDWriteData(MY_WORD_SWAP(usX));
    LCDWriteData(MY_WORD_SWAP(usY));
    LCDWriteData(MY_WORD_SWAP(usW));
    LCDWriteData(MY_WORD_SWAP(usH));
    LCDWriteData(MY_WORD_SWAP(usDpyMode));
    spi_device_release_bus(spi);
}

//-----------------------------------------------------------
//Host controller function 5 ¡V Write command to host data Bus with aruments
//-----------------------------------------------------------
void LCDSendCmdArg(uint16_t usCmdCode,uint16_t* pArg, uint16_t usNumArg){
     uint16_t i;
     //Send Cmd code
     LCDWriteCmdCode(usCmdCode);
     //Send Data
     for(i=0;i<usNumArg;i++){
         LCDWriteData(MY_WORD_SWAP(pArg[i]));
     }
}

//-----------------------------------------------------------
//Host Cmd 11 - LD_IMG_AREA
//-----------------------------------------------------------
void IT8951LoadImgAreaStart(struct IT8951LdImgInfo* pstLdImgInfo , struct IT8951AreaImgInfo* pstAreaImgInfo){
    uint16_t usArg[5];
    //Setting Argument for Load image start
    usArg[0] = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    usArg[1] = pstAreaImgInfo->usX;
    usArg[2] = pstAreaImgInfo->usY;
    usArg[3] = pstAreaImgInfo->usWidth;
    usArg[4] = pstAreaImgInfo->usHeight;
    //Send Cmd and Args
    LCDSendCmdArg(IT8951_TCON_LD_IMG_AREA , usArg , 5);
}


void IT8951LoadImgEnd(void){
    LCDWriteCmdCode(IT8951_TCON_LD_IMG_END);
}
// void IT8951HostAreaPackedPixelWrite4BPPWithPacket(struct IT8951LdImgInfo* pstLdImgInfo, struct IT8951AreaImgInfo* pstAreaImgInfo){
//     //Source buffer address of Host
//     spi_device_acquire_bus(spi, portMAX_DELAY);
//
//     //Set Image buffer(IT8951) Base address
//     IT8951SetImgBufBaseAddr();
//     //Send Load Image start Cmd
//     IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
//     //Host Write Data
//
//     LCDWriteNData(pstLdImgInfo->ulStartFBAddr, pstAreaImgInfo->usHeight * pstAreaImgInfo->usWidth/2);
//     //Send Load Img End Command
//     IT8951LoadImgEnd();
//     spi_device_release_bus(spi);
// }

void IT8951HostAreaPackedPixelWriteWifi(struct IT8951LdImgInfo* pstLdImgInfo, struct IT8951AreaImgInfo* pstAreaImgInfo){
    //Source buffer address of Host
    spi_device_acquire_bus(spi, portMAX_DELAY);

    //Set Image buffer(IT8951) Base address
    IT8951SetImgBufBaseAddr();

    //Send Load Image start Cmd
    IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);

    //Host Write Data
    LCDWriteWifiPackets();
    //Send Load Img End Command
    IT8951LoadImgEnd();
    spi_device_release_bus(spi);
}

// void IT8951HostAreaPackedPixelWrite4BPP(struct IT8951LdImgInfo* pstLdImgInfo, struct IT8951AreaImgInfo* pstAreaImgInfo){
//     //Source buffer address of Host
//     spi_device_acquire_bus(spi, portMAX_DELAY);
//
//     //Set Image buffer(IT8951) Base address
//     IT8951SetImgBufBaseAddr();
//     //Send Load Image start Cmd
//     IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
//     //Host Write Data
//     //0 is black F is white
//     //00 resulted in all black
//     //0f resulted in first white then black verticle lines
//     LCDWriteNDataWithValue(0xFF, pstAreaImgInfo->usHeight * pstAreaImgInfo->usWidth/2);
//     //Send Load Img End Command
//     IT8951LoadImgEnd();
//     spi_device_release_bus(spi);
// }

void IT8951BlankDisplay(){
    IT8951WaitForDisplayReady();
    IT8951DisplayArea(0,0, displayInfo.info.usPanelW, displayInfo.info.usPanelH, 0);
}

void IT8951PicDisplayWifi(){
    IT8951WaitForDisplayReady();

    struct IT8951LdImgInfo stLdImgInfo;
    //stLdImgInfo.ulStartFBAddr    = NULL;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_B_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_4BPP;
    stLdImgInfo.usRotate         = IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = displayInfo.info.usImgBufAddrL | ((uint32_t)displayInfo.info.usImgBufAddrH << 16);

    struct IT8951AreaImgInfo stAreaImgInfo;
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = displayInfo.info.usPanelW;
    stAreaImgInfo.usHeight = displayInfo.info.usPanelH;

    //Load Image from Host to IT8951 Image Buffer
    IT8951HostAreaPackedPixelWriteWifi(&stLdImgInfo, &stAreaImgInfo);//Display function 2
    IT8951BlankDisplay();
    IT8951DisplayArea(stAreaImgInfo.usX,stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight, 2);
   ESP_LOGI(TAG_SPI, "IMAGE iS WRITTEN");
}


// void IT8951ShadeDisplay4BPP(){
//     IT8951WaitForDisplayReady();
//
//     struct IT8951LdImgInfo stLdImgInfo;
//     stLdImgInfo.ulStartFBAddr    = NULL;
//     stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
//     stLdImgInfo.usPixelFormat    = IT8951_4BPP;
//     stLdImgInfo.usRotate         = IT8951_ROTATE_0;
//     stLdImgInfo.ulImgBufBaseAddr = displayInfo.info.usImgBufAddrL | (displayInfo.info.usImgBufAddrH << 16);
//
//     struct IT8951AreaImgInfo stAreaImgInfo;
//     stAreaImgInfo.usX      = 0;
//     stAreaImgInfo.usY      = 0;
//     stAreaImgInfo.usWidth  = displayInfo.info.usPanelW;
//     stAreaImgInfo.usHeight = displayInfo.info.usPanelH;
//
//     //Load Image from Host to IT8951 Image Buffer
//     IT8951HostAreaPackedPixelWrite4BPP(&stLdImgInfo, &stAreaImgInfo);//Display function 2
//     IT8951DisplayArea(stAreaImgInfo.usX,stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight, 2);
// }

void display_task(void *pvParameter){
    while(1){
        if (displayFlag){
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            ESP_LOGI(TAG_SPI, "Display Idle");
            continue;
        }
        gpio_set_level(ENABLE_5V, 1);
        initDisplay();
        lcd_get_id();
        IT8951WriteReg(I80CPCR, 0x0100); //0100 is enable
        vcom_get();
        vcom_set();
        vcom_get();
        IT8951WaitForDisplayReady();
        // IT8951BlankDisplay();
        IT8951PicDisplayWifi();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        gpio_set_level(ENABLE_5V, 0);
        displayFlag = 1;
    }
}





#endif //}__DISPLAY_H__
