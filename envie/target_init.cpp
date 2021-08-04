#include "mbed.h"
#include "target_init.h"

// clock source is selected with CLOCK_SOURCE in json config
#define USE_PLL_HSE_EXTC     0x8  // Use external clock (ST Link MCO)
#define USE_PLL_HSE_XTAL     0x4  // Use external xtal (X3 on board - not provided by default)
#define USE_PLL_HSI          0x2  // Use HSI internal clock

volatile const uint8_t bootloader_data[] __attribute__ ((section (".bootloader_version"), used)) = {
  BOOTLOADER_CONFIG_MAGIC,
  BOOTLOADER_VERSION,
  CLOCK_SOURCE,
  PORTENTA_USB_SPEED,
  PORTENTA_HAS_ETHERNET,
  PORTENTA_HAS_WIFI,
  PORTENTA_RAM_SIZE,
  PORTENTA_QSPI_SIZE,
  PORTENTA_HAS_VIDEO,
  PORTENTA_HAS_CRYPTO,
  PORTENTA_EXTCLOCK,
};

static bool double_tap_flag = true;
volatile uint8_t ledKeepValue = 0;
volatile uint8_t ledTargetValue = 20;
volatile int8_t ledDirection = 1;
volatile int divisor = 0;

RTC_HandleTypeDef RtcHandle;

DigitalOut led(PK_6);

static inline void LED_pulse(DigitalOut* led)
{
  if (divisor++ % 40) {
    return;
  }

  if (HAL_GetTick() > 500 && double_tap_flag && HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0) == 0xDF59) {
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 0);
    double_tap_flag = false;
  }

  if (ledKeepValue == 0) {
    ledTargetValue += ledDirection;
    *led = !*led;
  }
  ledKeepValue ++;

  if (ledTargetValue > 250 || ledTargetValue < 10) {
    ledDirection = -ledDirection;
    ledTargetValue += ledDirection;
  }

  if (ledKeepValue == ledTargetValue) {
    *led = !*led;
  }
}

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
  }
}

int target_init(void) {
  DigitalIn boot_sel(PI_8,PullDown);

  HAL_RTC_MspInit(&RtcHandle);
  RTC_CalendarBkupInit();

  int magic = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0);
  printf("Magic 0x%x\n");

  // in case we have been reset let's wait 500 ms to see if user is trying to stay in bootloader
  if (ResetReason::get() == RESET_REASON_PIN_RESET) {
    // now that we've been reset let's set magic. resetting with this set will
    // flag we need to stay in bootloader.
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 0xDF59);
    HAL_Delay(500);
    printf("envie loader magic set 0x%x \n", HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0));
  }

  DigitalOut usb_reset(PJ_4, 0);
  DigitalOut video_enable(PJ_2, 0);
  DigitalOut video_reset(PJ_3, 0);

  Ticker pulse;
  //DigitalOut eth_rst(PJ_15, 1);

  HAL_FLASH_Unlock();

  I2C i2c(PB_7, PB_6);

  char data[2];

  // LDO2 to 1.8V
  data[0]=0x4F;
  data[1]=0x0;
  i2c.write(8 << 1, data, sizeof(data));
  data[0]=0x50;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));

  // LDO1 to 1.0V
  data[0]=0x4c;
  data[1]=0x5;
  i2c.write(8 << 1, data, sizeof(data));
  data[0]=0x4d;
  data[1]=0x3;
  i2c.write(8 << 1, data, sizeof(data));

  // LDO3 to 1.2V
  data[0]=0x52;
  data[1]=0x9;
  i2c.write(8 << 1, data, sizeof(data));
  data[0]=0x53;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));

  HAL_Delay(10);

  data[0]=0x9C;
  data[1]=(1 << 7);
  i2c.write(8 << 1, data, sizeof(data));

  // Disable charger led
  data[0]=0x9E;
  data[1]=(1 << 5);
  i2c.write(8 << 1, data, sizeof(data));

  HAL_Delay(10);

  // SW3: set 2A as current limit
  // Helps keeping the rail up at wifi startup
  data[0]=0x42;
  data[1]=(2);
  i2c.write(8 << 1, data, sizeof(data));

  HAL_Delay(10);

  // Change VBUS INPUT CURRENT LIMIT to 1.5A
  data[0]=0x94;
  data[1]=(20 << 3);
  i2c.write(8 << 1, data, sizeof(data));

#if 1
  // SW2 to 3.3V (SW2_VOLT)
  data[0]=0x3B;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));

  // SW1 to 3.0V (SW1_VOLT)
  data[0]=0x35;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));

  //data[0]=0x36;
  //data[1]=(2);
  //i2c.write(8 << 1, data, sizeof(data));
#endif

  HAL_Delay(10);

  usb_reset = 1;
  HAL_Delay(10);
  usb_reset = 0;
  HAL_Delay(10);
  usb_reset = 1;

  HAL_Delay(10);

  /* Test if user code is programmed starting from USBD_DFU_APP_DEFAULT_ADD
   * address. TODO check MCUBoot header instead.
   */
  int app_valid = (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x20000000)
               || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x24000000)
               || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x30000000)
               || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x38000000);

  if (app_valid && magic != 0xDF59 && magic != 0x07AA && boot_sel==0) {
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, 0);
    HAL_FLASH_Lock();
    printf("boot app magic 0x%x \n", HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0));
    return 0;

  } else {
    printf("boot loop magic 0x%x \n", HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0));
    return 1;
  }
}

void envie_loop(void) {
  while(1) {
    LED_pulse(&led);
  }
}