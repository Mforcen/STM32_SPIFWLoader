#include "bootloader.h"

uint8_t chksum_calc(uint8_t* pData, int len)
{
	uint8_t chksum = pData[0];
	for(int idx = 1; idx < len; ++idx) chksum ^= pData[idx];
	return chksum;
}

uint8_t bootloader_start()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	char send[] = {0x5A, 0x00};
	char recv;
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
	char trx=0;
	HAL_SPI_Transmit(&hspi1, &trx, 1, 1000);
	HAL_SPI_Receive(&hspi1, &trx, 1, 1000);
	if(trx == 0x79)
	{
		HAL_SPI_Transmit(&hspi1, &trx, 1, 1000);
		return 1;
	}
	return 0;
}

uint8_t bootloader_get_command()
{
	char to_send[128] = {0};
	char to_recv[128] = {0};

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

void bootloader_read(uint32_t addr, char* recvDataPtr, uint8_t len)
{
	char to_send[5];

	to_send[0] = 0x5A;
	to_send[1] = 0x11;
	to_send[2] = 0xEE;
	HAL_SPI_Transmit(&hspi1, to_send, 3, 1000);

	bootloader_get_ack();

	to_send[0] = (addr>>24)&0xFF;
	to_send[1] = (addr>>16)&0xFF;
	to_send[2] = (addr>>8)&0xFF;
	to_send[3] = addr&0xFF;
	to_send[4] = chksum_calc(to_send, 4);
	HAL_SPI_Transmit(&hspi1, to_send, 5, 1000);

	bootloader_get_ack();

	to_send[0] = 0x7F;
	to_send[1] = ~to_send[0];
	HAL_SPI_Transmit(&hspi1, to_send, 2, 1000);

	bootloader_get_ack();

	HAL_SPI_Transmit(&hspi1, to_send, 1, 1000);
	HAL_SPI_Receive(&hspi1, recvDataPtr, 128, 1000);
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

void bootloader_stop()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
