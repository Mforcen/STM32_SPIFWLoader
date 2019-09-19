#ifndef BOOTLOADER_H_INCLUDED
#define BOOTLOADER_H_INCLUDED

#include "main.h"

extern SPI_HandleTypeDef hspi1;

typedef struct
{
	uint8_t command;
	uint8_t pLength;
	uint32_t mem_addr;
    uint8_t* params;
} cmd_item;

#define CMDSTO_SIZE 32
extern volatile cmd_item cmd_storage[CMDSTO_SIZE];
extern volatile uint8_t cmd_head;
extern volatile uint8_t cmd_tail;

void init_cmd_storage();
uint8_t push_cmd(cmd_item item);
cmd_item pop_cmd();

uint8_t chksum_calc(uint8_t* pData, int len);
uint8_t bootloader_start();
uint8_t bootloader_get_ack();
uint8_t bootloader_get_command();
void bootloader_read(uint32_t addr, uint8_t* recvDataPtr, uint8_t len);
uint8_t bootloader_write(uint32_t addr, uint8_t* sendData, unsigned int len);
void bootloader_stop();

#endif // BOOTLOADER_H_INCLUDED
