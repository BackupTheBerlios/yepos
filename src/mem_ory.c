/*yepos the PalmOS dictionary: memory handling*/
#include<PalmOS.h>
#include"mem_ory.h"
#include"../include/signs.h"
#define mem_db_type ('Cach')
enum local_enums{mem_handles_num=0x11,min_chunk_size=4};
enum debug_constants{facunde=0};
static const char db_name[]="/var/lib/yepos/malloc";
static DmOpenRef db;
static MemHandle busy_handles[mem_handles_num];
static int
create_db(void)
{DmSearchStateType ss;UInt16 card=0,i;LocalID id;
 if(DmCreateDatabase(card,db_name,CREATOR,mem_db_type,0))return!0;
 if(DmGetNextDatabaseByTypeCreator(1,&ss,mem_db_type,CREATOR,0,&card,&id))
  return!0;
 db=DmOpenDatabase(0,id,dmModeReadWrite);if(!db)return!0;
 for(i=0;i<mem_handles_num;i++)DmNewRecord(db,&i,min_chunk_size);return 0;
}
static void
shrink_records(void)
{int i;for(i=0;i<mem_handles_num;i++)DmResizeRecord(db,i,min_chunk_size);}
int
init_memory(void)
{unsigned long i;DmSearchStateType ss;UInt16 card;LocalID id;UInt32 rec_num;
 if(DmGetNextDatabaseByTypeCreator(1,&ss,mem_db_type,CREATOR,0,&card,&id))
  return create_db();
 if(DmDatabaseSize(card,id,&rec_num,0,0))return!0;
 db=DmOpenDatabase(0,id,dmModeReadWrite);if(!db)return!0;
 if(rec_num>mem_handles_num)for(i=rec_num-1;i>=mem_handles_num;i++)
  DmRemoveRecord(db,i);
 shrink_records();return 0;
}
static void
draw_chars(const char*s)
{if(facunde){WinDrawChars(s,StrLen(s),0,0);SysTaskDelay(50);}}
void
close_memory()
{int i;
 for(i=0;i<mem_handles_num;i++)if(busy_handles[i])
 {struct mem_chunk m;draw_chars("unfreed handle");m.d=i;free_chunk(m);}
 if(!db)return;DmCloseDatabase(db);
}struct mem_chunk
alloc_chunk(unsigned size)
{struct mem_chunk m;int i;m.d=invalid_chunk_descriptor;
 for(i=0;i<mem_handles_num&&busy_handles[i];i++);
 if(!db){draw_chars("no db");m.d=no_chunk_db;return m;}
 if(i==mem_handles_num)
 {draw_chars("all busy");m.d=all_descriptors_busy;return m;}
 if(!(busy_handles[i]=DmResizeRecord(db,i,size)))
 {draw_chars("can't resize  ");m.d=chunk_resize_failed;return m;}
 m.d=i;m.p=MemHandleLock(busy_handles[i]);if(!m.p)m.d=handle_lock_failed;
 if(facunde)
 {char s[0x33];StrCopy(s,"alloc_chunk ");StrIToA(s+StrLen(s),m.d);StrCat(s,"   ");
  draw_chars(s);
 }return m;
}
const char*
lock_chunk(struct mem_chunk m)
{MemHandleUnlock(busy_handles[m.d]);DmReleaseRecord(db,m.d,0);
 busy_handles[m.d]=DmQueryRecord(db,m.d);if(!busy_handles[m.d])return 0;
 return MemHandleLock(busy_handles[m.d]);
}void
free_chunk(struct mem_chunk p)
{mem_chunk_descriptor d=p.d;
 if(facunde)
 {char s[0x33];StrCopy(s,"free_chunk ");StrIToA(s+StrLen(s),d);StrCat(s,"   ");
  draw_chars(s);
 }if(d>=mem_handles_num||d<0)return;
 if(busy_handles[d]){MemHandleUnlock(busy_handles[d]);busy_handles[d]=0;}
 DmResizeRecord(db,d,min_chunk_size);
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
