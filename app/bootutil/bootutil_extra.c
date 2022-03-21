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

#include "bootutil_extra.h"
#include "rtc.h"

int boot_set_debug(int enable) {
  RTCInit();
  unsigned int rtc_reg = RTCGetBKPRegister(RTC_BKP_DR7);
  
  if(enable) {
    rtc_reg |= 0x00000001;
  } else {
    rtc_reg &= ~0x00000001;
  }
  
  return RTCSetBKPRegister(RTC_BKP_DR7, rtc_reg);
}
