/**
  ******************************************************************************
  * @file    USB_Device/DFU_Standalone/Src/usbd_dfu_flash.c
  * @author  MCD Application Team
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#if MCUBOOT_AS_ENVIE && MCUBOOT_ENVIE_DFU

/* Includes ------------------------------------------------------------------ */
#include "usbd_dfu_flash.h"
//#include "option_bits.h"
#include "mbed.h"
#include "target_init.h"
#include "QSPIFBlockDevice.h"
#include "FlashSimBlockDevice.h"
#include "flash_map_backend/secondary_bd.h"
#include "bootutil/bootutil.h"

/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
#define FLASH_DESC_STR      "@Internal Flash  2MB   /0x08000000/01*128Ka,15*128Kg"
//#define BOOTLOADER_DESC_STR    "@Option Bits      /0x52002000/01*1Ka"
#define QSPI_FLASH_DESC_STR "@Ext RAW  Flash 16MB   /0x90000000/4096*4Kg"
#define FILE_FLASH_DESC_STR "@Ext File Flash 16MB   /0xA0000000/16*128Kg"

#define FLASH_ERASE_TIME    (uint16_t)0
#define FLASH_PROGRAM_TIME  (uint16_t)0

/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
/* Private function prototypes ----------------------------------------------- */

char BOOTLOADER_DESC_STR[48];


/* Extern function prototypes ------------------------------------------------ */
uint16_t Flash_If_Init(void);
uint16_t Flash_If_Erase(uint32_t Add);
uint16_t Flash_If_Write(uint8_t * src, uint8_t * dest, uint32_t Len);
uint8_t *Flash_If_Read(uint8_t * src, uint8_t * dest, uint32_t Len);
uint16_t Flash_If_DeInit(void);
uint16_t Flash_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t * buffer);

FlashIAP flash;
QSPIFBlockDevice qspi_flash(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
mbed::BlockDevice* dfu_secondary_bd = get_secondary_bd();

const uint32_t QSPIFLASH_BASE_ADDRESS   =  0x90000000;
const uint32_t FILEBLOCK_BASE_ADDRESS   =  0xA0000000;

USBD_DFU_MediaTypeDef USBD_DFU_Flash_fops = {
  {
		  (uint8_t *) FLASH_DESC_STR,
		  (uint8_t *) QSPI_FLASH_DESC_STR,
      (uint8_t *) FILE_FLASH_DESC_STR,
      (uint8_t *) BOOTLOADER_DESC_STR
  },
  Flash_If_Init,
  Flash_If_DeInit,
  Flash_If_Erase,
  Flash_If_Write,
  Flash_If_Read,
  Flash_If_GetStatus,
};

bool Flash_If_Init_requested = false;

void init_Memories() {
  flash.init();
  qspi_flash.init();
  dfu_secondary_bd->init();
  snprintf(BOOTLOADER_DESC_STR, sizeof(BOOTLOADER_DESC_STR), "@MCUBoot version %d /0x00000000/0*4Kg", BOOTLOADER_VERSION);
}

Thread writeThread(osPriorityHigh);
EventQueue writeQueue;

/**
  * @brief  Initializes Memory.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t Flash_If_Init(void)
{
  Flash_If_Init_requested = true;
  writeThread.start(callback(&writeQueue, &EventQueue::dispatch_forever));
  return 0;
}

/**
  * @brief  De-Initializes Memory.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t Flash_If_DeInit(void)
{
  flash.deinit();
  dfu_secondary_bd->deinit();
  boot_set_pending(false);
  return 0;
}

static bool isExternalFlash(uint32_t Add) {
	return (Add >= QSPIFLASH_BASE_ADDRESS);
}

static bool isFileBlockFlash(uint32_t Add) {
	return (Add >= FILEBLOCK_BASE_ADDRESS);
}

/**
  * @brief  Erases sector.
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t Flash_If_Erase(uint32_t Add)
{
  if (isFileBlockFlash(Add)) {
    Add -= FILEBLOCK_BASE_ADDRESS;
    return dfu_secondary_bd->erase(Add, dfu_secondary_bd->get_erase_size(Add));
  } else if (isExternalFlash(Add)) {
    Add -= QSPIFLASH_BASE_ADDRESS;
    return qspi_flash.erase(Add, qspi_flash.get_erase_size(Add));
  } else {
    return flash.erase(Add, flash.get_sector_size(Add));
  }
}

struct writeInfo {
  uint8_t* src;
  uint8_t* dest;
  uint32_t Len;
};

void delayed_write(struct writeInfo* info) {
  flash.program(info->src, (uint32_t)info->dest, info->Len);
  free(info->src);
  free(info);
}

/**
  * @brief  Writes Data into Memory.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t Flash_If_Write(uint8_t * src, uint8_t * dest, uint32_t Len)
{
  if (isFileBlockFlash((uint32_t)dest)) {
    dest -= FILEBLOCK_BASE_ADDRESS;
    if (Len < dfu_secondary_bd->get_erase_size(0)) {
      Len = dfu_secondary_bd->get_erase_size(0);
    }
    return dfu_secondary_bd->program(src, (uint32_t)dest, Len);
  } else if (isExternalFlash((uint32_t)dest)) {
    dest -= QSPIFLASH_BASE_ADDRESS;
    if (Len < qspi_flash.get_erase_size(0)) {
      Len = qspi_flash.get_erase_size(0);
    }
    return qspi_flash.program(src, (uint32_t)dest, Len);
  } else {
    uint8_t* srcCopy = (uint8_t*)malloc(Len);
    memcpy(srcCopy, src, Len);
    struct writeInfo* info = (struct writeInfo*)malloc(sizeof(struct writeInfo));
    info->src = srcCopy;
    info->dest = dest;
    info->Len = Len;
    writeQueue.call(callback(&delayed_write, info));
  }
  return 0;
}

/**
  * @brief  Reads Data into Memory.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *Flash_If_Read(uint8_t * src, uint8_t * dest, uint32_t Len)
{
  uint32_t i = 0;
  uint8_t *psrc = src;

  if (isFileBlockFlash((uint32_t)src)) {
    src -= FILEBLOCK_BASE_ADDRESS;
    dfu_secondary_bd->read(dest, (uint32_t)src, Len);
  } else if (isExternalFlash((uint32_t)src)) {
    src -= QSPIFLASH_BASE_ADDRESS;
    qspi_flash.read(dest, (uint32_t)src, Len);
  } else {
    for (i = 0; i < Len; i++)
    {
      dest[i] = *psrc++;
    }
  }

  /* Return a valid address to avoid HardFault */
  return (uint8_t *) (dest);
}

/**
  * @brief  Gets Memory Status.
  * @param  Add: Address to be read from.
  * @param  Cmd: Number of data to be read (in bytes).
  * @retval 0 if operation is successful
  */
uint16_t Flash_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t * buffer)
{
  switch (Cmd)
  {
  case DFU_MEDIA_PROGRAM:
    buffer[1] = (uint8_t) FLASH_PROGRAM_TIME;
    buffer[2] = (uint8_t) (FLASH_PROGRAM_TIME >> 8);
    buffer[3] = 0;
    break;

  case DFU_MEDIA_ERASE:
  default:
    buffer[1] = (uint8_t) FLASH_ERASE_TIME;
    buffer[2] = (uint8_t) (FLASH_ERASE_TIME >> 8);
    buffer[3] = 0;
    break;
  }
  return 0;
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
