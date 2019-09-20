#include "bootloader.h"

volatile cmd_item cmd_storage[32]={0};
volatile uint8_t cmd_head;
volatile uint8_t cmd_tail;

void init_cmd_storage()
{
	cmd_head = 0;
	cmd_tail = 0;
}

uint8_t push_cmd(cmd_item item)
{
	uint8_t next_head = (cmd_head+1)%CMDSTO_SIZE;
	if(next_head == cmd_tail) return 1;

	cmd_storage[cmd_head].command = item.command;
	cmd_storage[cmd_head].pLength = item.pLength;
	//cmd_storage[cmd_head].params = item.params;
	for(uint8_t idx = 0; idx < 128; ++idx) cmd_storage[cmd_head].params[idx] = item.params[idx];
	cmd_storage[cmd_head].mem_addr = item.mem_addr;
	cmd_head = next_head;
	return 0;
}

cmd_item pop_cmd()
{
	if(cmd_head == cmd_tail)
	{
		cmd_item retval;
		retval.command = 0;
		retval.pLength = 0;
		retval.mem_addr = 0;
		return retval;
	}
	else
	{
		uint8_t old_tail = cmd_tail;
		cmd_tail = (cmd_tail+1)%CMDSTO_SIZE;
		return cmd_storage[old_tail];
	}
}

uint8_t chksum_calc(uint8_t* pData, int len)
{
	uint8_t chksum = pData[0];
	for(int idx = 1; idx < len; ++idx) chksum ^= pData[idx];
	return chksum;
}

uint8_t bootloader_start()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	uint8_t send[] = {0x5A, 0x00};
	uint8_t recv;
	uint8_t trials = 3;
	HAL_SPI_Transmit(&hspi1, send, 2, 1000);
	HAL_SPI_Receive(&hspi1, &recv, 1, 1000);
	while(recv != 0x79 && trials-->0)
	{
		HAL_SPI_Transmit(&hspi1, &recv, 1, 1000);
	}
	HAL_SPI_Transmit(&hspi1, &recv, 1, 1000);//dummy
	return recv!=0x79;
}

uint8_t bootloader_get_ack()
{
	uint8_t trx=0, tries = 0;
	HAL_SPI_Transmit(&hspi1, &trx, 1, 1000);
	do
	{
		HAL_SPI_Receive(&hspi1, &trx, 1, 1000);
		tries++;
		HAL_Delay(5);
	} while(trx != 0x79 && trx != 0x1F && tries < 5);
	if(trx == 0x79 || trx == 0x1F)
	{
		HAL_SPI_Transmit(&hspi1, &trx, 1, 1000);
		return 1;
	}
	return 0;
}

uint8_t bootloader_get_command()
{
	uint8_t to_send[128] = {0};
	uint8_t to_recv[128] = {0};

	to_send[0] = 0x5A;
	to_send[1] = 0x00;
	to_send[2] = 0xff;
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	if(bootloader_get_ack())
	{
		HAL_SPI_Receive(&hspi1, to_recv, 14, 1000);
		return 0;
	}
	return 1;
}

uint8_t bootloader_read(uint32_t addr, uint8_t* recvDataPtr, uint8_t len)
{
	uint8_t to_send[5];

	to_send[0] = 0x5A;
	to_send[1] = 0x11;
	to_send[2] = 0xEE;
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	if(!bootloader_get_ack()) return 1;

	to_send[0] = (addr>>24)&0xFF;
	to_send[1] = (addr>>16)&0xFF;
	to_send[2] = (addr>>8)&0xFF;
	to_send[3] = addr&0xFF;
	to_send[4] = chksum_calc(to_send, 4);
	HAL_SPI_Transmit(&hspi1, to_send, 5, 1000);

	if(!bootloader_get_ack()) return 2;

	to_send[0] = len;
	to_send[1] = ~to_send[0];
	HAL_SPI_Transmit(&hspi1, to_send, 2, 1000);

	if(!bootloader_get_ack()) return 3;

	HAL_SPI_Transmit(&hspi1, to_send, 1, 1000);
	HAL_SPI_Receive(&hspi1, recvDataPtr, len, 1000);

	return 0;
}

uint8_t bootloader_go(uint32_t addr)
{
	uint8_t to_send[5] = {0x5A, 0x21, 0xDE, 0x00, 0x00};
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	if(!bootloader_get_ack()) return 1;

	to_send[0] = (addr>>24)&0xFF;
	to_send[1] = (addr>>16)&0xFF;
	to_send[2] = (addr>>8)&0xFF;
	to_send[3] = addr&0xFF;
	to_send[4] = chksum_calc(to_send, 4);

	HAL_SPI_Transmit(&hspi1, to_send, 5, 1000);

	if(!bootloader_get_ack()) return 2;
	return 0;
}

uint8_t bootloader_write(uint32_t addr, uint8_t* sendData, unsigned int len)
{
    HAL_GPIO_WritePin(GPIOE, LD5_Pin|LD7_Pin|LD9_Pin|LD10_Pin, GPIO_PIN_SET);
	uint8_t to_send[130];

	to_send[0] = 0x5A;
	to_send[1] = 0x31;
	to_send[2] = 0xCE;
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	if(!bootloader_get_ack()) return 1;

	to_send[0] = (addr>>24)&0xFF;
	to_send[1] = (addr>>16)&0xFF;
	to_send[2] = (addr>>8)&0xFF;
	to_send[3] = addr&0xFF;
	to_send[4] = chksum_calc(to_send, 4);
	HAL_SPI_Transmit(&hspi1, to_send, 5, 1000);

	if(!bootloader_get_ack()) return 2;

	to_send[0] = len-1;

	unsigned int i;
	for(i = 0; i < len; ++i)
	{
        to_send[i+1] = sendData[i];
	}

	HAL_GPIO_WritePin(GPIOE, LD5_Pin, GPIO_PIN_RESET);
	//to_send[len+1] = chksum_calc(to_send, len+1);
	uint8_t chksum = to_send[0];
	for(int idx = 1; idx < (len+1); ++idx) chksum ^= to_send[idx];
	to_send[len+1] = chksum;
	HAL_GPIO_WritePin(GPIOE, LD7_Pin, GPIO_PIN_RESET);

	HAL_Delay(2);

	HAL_SPI_Transmit(&hspi1, to_send, len+2, 1000);

	HAL_GPIO_WritePin(GPIOE, LD9_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, to_send, 1, 1000);
	HAL_Delay(2);

	HAL_GPIO_WritePin(GPIOE, LD10_Pin, GPIO_PIN_RESET);
	if(!bootloader_get_ack()) return 3;
	return 0;
}

uint8_t bootloader_erase(uint16_t num, uint16_t* pages)
{
	/*uint8_t to_send[3] = {0x5A, 0x44, 0xBB};
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);
	if(!bootloader_get_ack()) return 1;

	to_send[0] = (pages>>8) & 0xFF;
	to_send[1] = pages & 0xFF;
	to_send[2] = to_send[0]^to_send[1];
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	if(!bootloader_get_ack()) return 2;

	if( (pages>>4) != 0xFFF)
	{
		to_send[0] = 0;
		to_send[1] = code;
		to_send[2] = to_send[0]^to_send[1];

		HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);
		HAL_Delay(1);
		if(!bootloader_get_ack()) return 3;
	}*/
	return 0;
}

uint8_t bootloader_write_protect()
{
	return 255;
}

uint8_t bootloader_write_unprotect()
{
	uint8_t to_send[3] = {0x5A, 0x73, 0x8C};
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);
	HAL_Delay(10);

	if(!bootloader_get_ack()) return 1;
	if(!bootloader_get_ack()) return 2;
	return 0;
}

uint8_t bootloader_readout_protect()
{
	uint8_t to_send[3] = {0x5A, 0x82, 0x7D};
	uint8_t dummy = 0x00;

	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);
	HAL_Delay(10);

	if(!bootloader_get_ack()) return 1;
	if(!bootloader_get_ack()) return 2;
	return 0;
}

uint8_t bootloader_readout_unprotect()
{
	uint8_t to_send[3] = {0x5A, 0x92, 0x6D};

	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);
	HAL_Delay(10);

	if(!bootloader_get_ack()) return 1;
	if(!bootloader_get_ack()) return 2;
	return 0;
}

void bootloader_stop()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
