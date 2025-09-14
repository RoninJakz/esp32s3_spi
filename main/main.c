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
#define W_TX_PAYLOAD 0xA0   //1010 0000 
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2

//Register addresses
#define STATUS_REG 0x07         //0000 0111
#define RPD_REG    0x09         //0000 1001
#define FIFO_STATUS_REG 0x17    //0001 0111
#define CONFIG     0x00         //0000 0000 
#define RF_CH      0x05         //RF channel freq. s (F0 = 2400 + RF_CH_data [MHz]) 
#define TX_ADDR    0x10         //transmit address. (Reset value: 0xE7E7E7E7E7) RX_ADDR_P0 has to be equal to this address to handle auto_ack





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



    //set config bit PRIM_RX low -- transmission mode

    //Because of full-duplex protocol a dummybyte is required when reading
    //8 bit command -- 8 bit dummy to recieve STATUS 

    uint8_t regr = R_REGISTER | (CONFIG & 0x1F);  //masking the read command with config register map address
    uint8_t r_data; //reading data


    spi_transaction_t t;    //declare a transaction
    memset(&t, 0, sizeof(t));
    t.length =16;
    uint8_t sendbuf[8] = {regr,0xFF,0}; //creating a buffer to send data (0xFF dummy)
    uint8_t recvbuf[8] = {0}; //creating a buffer to recieve data
    t.tx_buffer = sendbuf; //sending
    t.rx_buffer = recvbuf; //receiving
    spi_device_transmit(handle, &t); //start transaction
    printf("\nTransmitted: 0x%02X%02X\n",sendbuf[0],sendbuf[1]);
    printf("Received: 0x%02X%02X\n",recvbuf[0],recvbuf[1]);
    r_data = recvbuf[1];

    uint8_t regw = W_REGISTER | (CONFIG & 0x1F); //masking the write command with config register map address
    uint8_t w_data = (r_data | 0x03); //command data, setting PWR_UP = 1, PRIM_RX = 1
    sendbuf[0] = regw; //write command
    sendbuf[1] = w_data; //write data
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
     
    
    //setup payload transaction

    sendbuf[0] = FLUSH_TX; //Flush Tx FIFO
    sendbuf[1] = 0xFF; //write data
    t.tx_buffer = sendbuf; 
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);
    
    sendbuf[0] = FLUSH_RX; //Flush Rx FIFO for auto_ack
    sendbuf[1] = 0xFF; //write data
    t.tx_buffer = sendbuf; 
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);

    sendbuf[0] = W_REGISTER | (RF_CH & 0x1F); //setting the transmission frequency
    //Note that the MSBit is only allowed to be 0. 
    // (F0 = 2400 + RF_CH_data [MHz]) 
    // RF_CH_data is represented by a 7 bit bin.number
    sendbuf[1] =  0x7F & 0000010;

    //setting the transmission address
    t.length = 48; 
    sendbuf[0] = W_REGISTER | (TX_ADDR & 0x1F);
    sendbuf[1] = 0xE7; //transmit address. Set RX_ADDR_P0 equal to this address to handle auto_ack
    sendbuf[2] = 0xE7;
    sendbuf[3] = 0xE7;
    sendbuf[4] = 0xE7;
    sendbuf[5] = 0xE7;


    //Creating and sending payload to nRF24L01+

    t.length = 40; 
    uint32_t TxPayload = 0XA1B2C3D4; // Transmission payload (max 32 bit). This will be transmitted by nRF24L01+

    sendbuf[0] = W_TX_PAYLOAD; 
    memcpy(&sendbuf[1], &TxPayload, sizeof(TxPayload)); //copy payload to sendbuf
    t.tx_buffer = sendbuf; 
    t.rx_buffer = recvbuf;
    spi_device_transmit(handle, &t);

    

        
 /*
    //config register olvasása


    

    printf("Register Data: 0x%02X\n",r_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    





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
        
    
    */

}
//0E 11:  0000 1110   0001 0001

// RX mode: PWR_UP: 1 PRIM_RX: 1 CS: 1

//F0= 2400 + RF_CH [MHz]
//You must program a transmitter and a receiver with the same RF channel frequency to communicate with
//each other.
