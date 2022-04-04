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

  // SW2 to 3.3V (SW2_VOLT)
  data[0]=0x3B;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));

  // SW1 to 3.0V (SW1_VOLT)
  data[0]=0x35;
  data[1]=0xF;
  i2c.write(8 << 1, data, sizeof(data));
}


void power_init() {
  portenta_power_init();
}