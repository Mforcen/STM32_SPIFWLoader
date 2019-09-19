#ifndef BOOTLOADER_H_INCLUDED
#define BOOTLOADER_H_INCLUDED

#include "main.h"

extern SPI_HandleTypeDef hspi1;

uint8_t bootloader_start();
uint8_t bootloader_get_ack();
uint8_t chksum_calc(uint8_t* pData, int len);
uint8_t bootloader_get_command();
void bootloader_read(uint32_t addr, char* recvDataPtr, uint8_t len);
uint8_t bootloader_write(uint32_t addr, uint8_t* sendData, unsigned int len);
void bootloader_stop();

#endif // BOOTLOADER_H_INCLUDED
