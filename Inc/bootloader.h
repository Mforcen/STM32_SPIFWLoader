#ifndef BOOTLOADER_H_INCLUDED
#define BOOTLOADER_H_INCLUDED

#include "main.h"

extern SPI_HandleTypeDef hspi1;

typedef struct
{
	uint8_t command;
	uint8_t pLength;
	uint32_t mem_addr;
    uint8_t params[128];
} cmd_item;

#define CMDSTO_SIZE 32
extern volatile cmd_item cmd_storage[CMDSTO_SIZE];
extern volatile uint8_t cmd_head;
extern volatile uint8_t cmd_tail;

void init_cmd_storage();
uint8_t push_cmd(cmd_item item);
cmd_item pop_cmd();

#define BOOTLOADER_ERASE_FLASH 0xFFFF

uint8_t chksum_calc(uint8_t* pData, int len);
uint8_t bootloader_start();
uint8_t bootloader_get_ack();
uint8_t bootloader_get_command();
uint8_t bootloader_read(uint32_t addr, uint8_t* recvDataPtr, uint8_t len);
uint8_t bootloader_go(uint32_t addr);
uint8_t bootloader_write(uint32_t addr, uint8_t* sendData, unsigned int len);
uint8_t bootloader_erase(uint16_t num, uint16_t* pages);
uint8_t bootloader_write_protect();
uint8_t bootloader_write_unprotect();
uint8_t bootloader_readout_protect();
uint8_t bootloader_readout_unprotect();
void bootloader_stop();

#endif // BOOTLOADER_H_INCLUDED
