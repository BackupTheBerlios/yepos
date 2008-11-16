/*yepos the PalmOS dictionary: battery indicator*/
#include"show_battery.h"
#include<PalmOS.h>
static RectangleType r={{1,0},{15,11}};
static int
battery_v20(char*s)
{unsigned v=SysBatteryInfoV20(0,0,0,0,0,0);
 if(v<1000)
 {int n=v/100;*s++=n+'0';v-=n*100;*s++='.';
  n=v/10;*s++=n+'0';v-=n*10;*s++=v+'0';
 }else*s++='*';*s++=0;return 2;
}static int
battery(char*s)
{UInt8 p;int r=5,n,p0;SysBatteryInfo(0,0,0,0,0,0,&p);p0=p;
 if(p0>=100){r=2;n=p/100;*s++=n+'0';p-=n*100;}
 if(p0>=10){n=p/10;*s++=n+'0';p-=n*10;}
 *s++=p+'0';*s++=0;return r;
}static int(*
get_battery)(char*)=battery_v20;
void
init_show_battery(void)
{UInt32 OSVer;
 if(FtrGet(sysFtrCreator,sysFtrNumROMVersion,&OSVer))return;
 if(OSVer>=sysMakeROMVersion(3,0,0,sysROMStageRelease,0))
  get_battery=battery;
}
void
show_battery(int now)
{char s[17];unsigned p;static unsigned long tic;
 if(!now&&TimGetTicks()<tic+200)return;tic=TimGetTicks();
 WinEraseRectangle(&r,0);p=get_battery(s);
 WinDrawChars(s,StrLen(s),p,0);WinDrawLine(19,0,19,10);
 WinDrawLine(20,3,21,3);WinDrawLine(20,7,21,7);WinDrawLine(22,3,22,7);
 WinDrawLine(0,0,19,0);WinDrawLine(0,10,19,10);WinDrawLine(0,0,0,10);
}/*Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.*/
