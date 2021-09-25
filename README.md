# STM32 SPIFWLoader

A firmware used to communicate with the bootloader of another STM32 through SPI.
Although there are some devices that can be used to program STM32 using the SPI bus, sometimes is handy flash another off the self board STM with USB capabilities to burn an image into another device.

This repo also contains a python script which can be used to control the programmer, send files, dump memory or execute an arbitrary address of memory.