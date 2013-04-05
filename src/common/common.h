#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "std.h"

#ifndef LIBOPENCM3_CM3_COMMON_H
//Older gcc does not allow typedef redefinition even to thesame type
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

#define SPIFLASH_SECTOR_OFFSET 0
#define SPIFLASH_SECTORS 512

/* SPI Flash */
void SPIFlash_Init();
u32  SPIFlash_ReadID();
void SPIFlash_EraseSector(u32 sectorAddress);
void SPIFlash_BulkErase();
void SPIFlash_WriteBytes(u32 writeAddress, u32 length, const u8 * buffer);
void SPIFlash_WriteByte(u32 writeAddress, const u8 byte);
void SPIFlash_ReadBytes(u32 readAddress, u32 length, u8 * buffer);
int  SPIFlash_ReadBytesStopCR(u32 readAddress, u32 length, u8 * buffer);
void SPI_FlashBlockWriteEnable(u8 enable);

/* USB*/
void USB_Enable(u8 use_interrupt);
void USB_Disable();
void USB_HandleISR();
void USB_Connect();

/* Filesystem */
int  FS_Mount();
void FS_Unmount();
int  FS_OpenDir(const char *path);
int  FS_ReadDir(char *path);
void FS_CloseDir();

#endif
