/*yepos the PalmOS dictionary: memory handling test*/
#include<PalmOS.h>
#include"mem_ory.h"
static int y;
static void
draw_chars(const char*s){WinDrawChars(s,StrLen(s),0,y);SysTaskDelay(50);}
static int
init(void){return init_memory();}
static void
close(void){close_memory();}
enum local_constants{desc=0x11,chunk_size=4913};
unsigned
access_array(const char*a)
{unsigned i,s=0;for(i=0;i<chunk_size;i++,a++)s|=*a;return s;}
UInt32
PilotMain(UInt16 cmd,void*params,UInt16 flags)
{struct mem_chunk d[desc];const char*p[desc];int i,j;
 char*u,s[0x33];unsigned long t;
 if(cmd!=sysAppLaunchCmdNormalLaunch)return 0;
 t=TimGetTicks();
 if(init()){draw_chars("can't init ");return 0;}
 t=TimGetTicks()-t;
 StrCopy(s,"init: ");StrIToA(s+StrLen(s),t);
 StrCat(s,"  ");draw_chars(s);
 if(!(u=MemPtrNew(chunk_size/0x11))){draw_chars("can't get new ptr  ");return 0;}
 draw_chars("MemPtrNew  ");
 for(i=0;i<chunk_size/0x11;i++)u[i]=i&0xFF;
 draw_chars("memory filled ");
 for(i=0;i<desc;i++)
 {d[i]=alloc_chunk(chunk_size);
  if(d[i].d<0)
  {StrCopy(s,"can't alloc ");StrIToA(s+StrLen(s),i);
   StrCat(s,"  ");draw_chars(s);
  }
  for(j=0;j<0x11;j++)if(write_chunk(d[i],j*chunk_size/0x11,u,chunk_size/0x11))
  {StrCopy(s,"write failed ");StrIToA(s+StrLen(s),i);
   StrCat(s,":");StrIToA(s+StrLen(s),j);StrCat(s,"  ");draw_chars(s);
   continue;
  }p[i]=lock_chunk(d[i]);
  for(j=0;j<chunk_size;j++)*u=p[i][j];
 }for(i=0;i<desc;i++)free_chunk(d[i]);
 t=TimGetTicks();
 for(i=0;i<1000;i++)
 {d[0]=alloc_chunk(chunk_size);
  if(d[0].d<0)
  {StrCopy(s,"can't alloc ");StrIToA(s+StrLen(s),i);
   StrCat(s,"  ");draw_chars(s);
  }
  for(j=0;j<0x11;j++)if(write_chunk(d[0],j*chunk_size/0x11,u,chunk_size/0x11))
  {StrCopy(s,"write failed ");StrIToA(s+StrLen(s),j);
   StrCat(s,"  ");draw_chars(s);break;
  }p[0]=lock_chunk(d[0]);
  for(j=0;j<chunk_size;j++)*u=p[0][j];
  free_chunk(d[0]);
 }
 t=TimGetTicks()-t;
 StrCopy(s,"copy*1000: ");StrIToA(s+StrLen(s),t);StrCat(s,"  ");draw_chars(s);
 MemPtrFree(u);u=MemPtrNew(chunk_size);y+=12;
 t=TimGetTicks();
 for(i=0;i<1000;i++)
 {d[0]=alloc_chunk(chunk_size);
  if(d[0].d<0)
  {StrCopy(s,"can't alloc ");StrIToA(s+StrLen(s),i);
   StrCat(s,"  ");draw_chars(s);
  }
  if(write_chunk(d[0],0,u,chunk_size))
  {StrCopy(s,"write failed ");StrIToA(s+StrLen(s),j);
   StrCat(s,"  ");draw_chars(s);break;
  }p[0]=lock_chunk(d[0]);
  free_chunk(d[0]);
 }
 t=TimGetTicks()-t;
 StrCopy(s,"copy*1000: ");StrIToA(s+StrLen(s),t);StrCat(s,"  ");draw_chars(s);
 y+=12;t=TimGetTicks();
 for(j=i=0;i<1000;i++)j|=access_array(u);
 t=TimGetTicks()-t;
 StrCopy(s,"dyn access*1000: ");StrIToA(s+StrLen(s),t);StrCat(s,"  ");draw_chars(s);
 y+=12;
 {d[0]=alloc_chunk(chunk_size);
  if(d[0].d<0)
  {StrCopy(s,"can't alloc ");StrIToA(s+StrLen(s),i);
   StrCat(s,"  ");draw_chars(s);
  }
  if(write_chunk(d[0],0,u,chunk_size))
  {StrCopy(s,"write failed ");StrIToA(s+StrLen(s),j);
   StrCat(s,"  ");draw_chars(s);
  }else
  {p[0]=lock_chunk(d[0]);
   t=TimGetTicks();
   for(j=i=0;i<1000;i++)j|=access_array(p[0]);
   t=TimGetTicks()-t;
   StrCopy(s,"strg access*1000: ");StrIToA(s+StrLen(s),t);StrCat(s,"  ");draw_chars(s);
   free_chunk(d[0]);
  }
 }y+=12;
 draw_chars("freeing ");
 MemPtrFree(u);
 draw_chars("freeing chunks ");
 draw_chars("closing ");
 close();
 draw_chars("send penDownEvent ");
 while(1)
 {EventType e;EvtGetEvent(&e,evtWaitForever);if(e.eType==penDownEvent)break;}
 return 0;
}/*Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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
