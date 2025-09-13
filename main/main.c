#include <stdio.h>
#include <string.h>

 #include "freertos/FreeRTOS.h"
 #include "driver/spi_master.h"
 #include "driver/gpio.h"
 #include "esp_mac.h"

 //SPI bus pins
 #define MOSI   12
 #define MISO   13
 #define SCLK   15
 #define CS     14

//Commands
//'A' represents the register map address
//To make a sensible command the command and reg. address bits have to be masked together
#define R_REGISTER 0x00     //000A AAAA
#define W_REGISTER 0x20     //001A AAAA

//Register addresses
#define STATUS_REG 0x07         //0000 0111
#define RPD_REG    0x09         //0000 1001
#define FIFO_STATUS_REG 0x17    //0001 0111
#define CONFIG     0x00         //0000 0000 








void app_main(void)
{
    //We create the device 'handle', which is a pointer pointing to the spi_device_t struct
    spi_device_handle_t handle;

    //SPI bus config. Here we add the common pins of the bus
    spi_bus_config_t spi_config = 
    {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = SCLK,

    };

    //Slave device config. We specify parameters related to a single slave device (eg. CS)
    spi_device_interface_config_t spi_device_config = 
    {
        .clock_speed_hz = 1000000,
        .duty_cycle_pos = 128,
        .mode           = 0,
        .spics_io_num   = CS,
        .queue_size     = 1
    };

    //Initialize bus
    spi_bus_initialize(SPI2_HOST,&spi_config,SPI_DMA_CH_AUTO);
    //Device init. A spi_device_t struct is allocated for the slave device. Returns the pointer of the structs pointer.
    spi_bus_add_device(SPI2_HOST,&spi_device_config,&handle);




    //Masking the command and address bits
    uint8_t regr = R_REGISTER | (CONFIG & 0x1F);     
    uint8_t regw = W_REGISTER | (CONFIG & 0x1F);

    //Writing data
    uint8_t w_data;
    //Reading data
    uint8_t r_data;

    //Because of full-duplex protocol a dummybyte is required
    //8 bit command -- 8 bit dummy to recieve STATUS 
    uint8_t sendbuf[2] = {regr,0xFF};
    uint8_t recvbuf[2] = {0};


    spi_transaction_t t;
    memset(&t, 0, sizeof(t));


    


    t.length =16;

        

    //config register olvasása
    sendbuf[0] = regr;
    sendbuf[1] = 0xFF;

    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
    printf("\nReading phase\n");
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);

    r_data = recvbuf[1];
    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
        

    //config register írása

    w_data = (r_data | 0x03); //PWR_UP = 1, PRIM_RX = 1
    sendbuf[0] = regw; 
    sendbuf[1] = w_data;

    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);

    printf("\nWriting phase\n");
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);

    r_data = recvbuf[1];
    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

            
                
    //config register visszaolvasása:

    sendbuf[0] = regr;
    sendbuf[1] = 0xFF;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
    printf("\nReading phase\n");
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);
            
    r_data = recvbuf[1];
    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    //CONFIG register visszaállítása 

    w_data = (r_data & 0xFC); //PWR_UP = 0; PRIM_RX = 0;
    sendbuf[0] = regw; 
    sendbuf[1] = w_data;

    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
    printf("\nWriting phase\n");
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);

    r_data = recvbuf[1];
    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);        

            
        
    sendbuf[0] = regr;
    sendbuf[1] = 0xFF;

    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
    printf("\nReading phase\n");
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);

    r_data = recvbuf[1];
    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
        
     
}
//0E 11:  0000 1110   0001 0001

// RX mode: PWR_UP: 1 PRIM_RX: 1 CS: 1

//F0= 2400 + RF_CH [MHz]
//You must program a transmitter and a receiver with the same RF channel frequency to communicate with
//each other.
