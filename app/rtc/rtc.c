/*
  Copyright (c) 2022 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if  MCUBOOT_APPLICATION_HOOKS

#include "rtc.h"
#include "bootutil/bootutil_log.h"

RTC_HandleTypeDef RtcHandle;
static int rtc_initialized = 0;

void RTC_Bkp_Init(void)
{
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
  - Hour Format    = Format 24
  - Asynch Prediv  = Value according to source clock
  - Synch Prediv   = Value according to source clock
  - OutPut         = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType     = Open Drain */
  RtcHandle.Instance = RTC;
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  rtc_initialized = 1;
}

void RTCInit() {
  if(!rtc_initialized) { 
    RTC_Bkp_Init();
  }
}

uint32_t RTCGetBKPRegister(uint32_t BackupRegister) {
    if(rtc_initialized) {
      return HAL_RTCEx_BKUPRead(&RtcHandle, BackupRegister);
    } else {
      BOOT_LOG_ERR("RTCGetBKPRegister");
      return -1;
    }
}

uint32_t RTCSetBKPRegister(uint32_t BackupRegister, uint32_t Data) {
    if(rtc_initialized) { 
      HAL_RTCEx_BKUPWrite(&RtcHandle, BackupRegister, Data);
      return 0;
    } else {
      BOOT_LOG_ERR("RTCSetBKPRegister");
      return -1;
    }
}

#endif
