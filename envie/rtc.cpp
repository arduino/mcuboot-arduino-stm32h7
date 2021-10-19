#if MCUBOOT_AS_ENVIE

#include "rtc.h"
#include "bootutil/bootutil_log.h"

RTC_HandleTypeDef RtcHandle;
static int rtc_initialized = 0;


void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Configure LSE as RTC clock source ##################################*/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    return;
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    return;
  }

  /*##-2- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

void RTC_CalendarBkupInit(void)
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
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    BOOT_LOG_ERR("HAL_RTC_Init");
    return;
  }

  rtc_initialized = 1;
}

void RTCInit() {
  if(!rtc_initialized) { 
    HAL_RTC_MspInit(&RtcHandle);
    RTC_CalendarBkupInit();
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
