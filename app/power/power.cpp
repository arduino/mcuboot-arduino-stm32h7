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

#include "power.h"
#include "board.h"
#include "mbed.h"

#if defined TARGET_PORTENTA_H7_M7
static void portenta_power_init() {
  I2C i2c(BOARD_I2C_SDA, BOARD_I2C_SCL);

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
  
  // At this point  in the start-up sequence, the DC/DC converter output voltages of the PF1550, power management IC (PMIC), 
  // have been configured with the values found in the one-time programmable registers, themselves written prior to the PMIC, during production.
  // On some commercial samples, these values were programmed incorrectly, and the PMIC outputs 3.0V instead of 3.1 or 3.3 on the respective rails.
  // Therefore, it is first necessary to turn the SW1 and SW2 rails off, program the correct values in the SWx_VOLT registers,
  // and then turn them back on using the SWx_VOLT_CTRL register for the changes to take full effect.

  // SW1 turn off before config (SW1_VOLT_CTRL)
  data[0]=0x35;
  data[1]=0x0;
  i2c.write(8 << 1, data, sizeof(data));

  // SW2 turn off before config (turned on by OTP settings) (SW2_CTRL)
  data[0]=0x3B;
  data[1]=0x0;
  i2c.write(8 << 1, data, sizeof(data));

  // SW1 set to 3.0V (SW1_VOLT)
  data[0]=0x32;
  data[1]=0x6;
  i2c.write(8 << 1, data, sizeof(data));

  // SW2 set to 3.3V (SW2_VOLT)
  data[0]=0x38;
  data[1]=0x7;
  i2c.write(8 << 1, data, sizeof(data));
  
  // SW1 turn back ON (SW1_CTRL)
  data[0]=0x35;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));
  
  // SW2 turn back ON (SW2_VOLT_CTRL)
  data[0]=0x3B;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));
}
#endif

#if defined TARGET_NICLA_VISION
static void nicla_vision_power_init() {
  I2C i2c(BOARD_I2C_SDA, BOARD_I2C_SCL);

  char data[2];

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
}
#endif

void power_init() {
#if defined TARGET_PORTENTA_H7_M7
  portenta_power_init();
#elif defined TARGET_NICLA_VISION
  nicla_vision_power_init();
#elif defined TARGET_GIGA
  //no power init function
#else

#endif
}
