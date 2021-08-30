#ifndef __RTC_H
#define __RTC_H

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rtc.h"

void RTCInit();
uint32_t RTCGetBKPRegister(uint32_t BackupRegister);
uint32_t RTCSetBKPRegister(uint32_t BackupRegister, uint32_t Data);

#endif //__RTC_H
