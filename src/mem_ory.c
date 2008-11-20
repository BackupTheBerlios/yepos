/*yepos the PalmOS dictionary: memory handling*/
#include<PalmOS.h>
#include"mem_ory.h"
#include"../include/signs.h"
#ifdef COUNT_MEMORY
static unsigned long max_used,used;
static long num_locked,max_locked;
void*
mem_new(unsigned long size)
{MemPtr p=MemPtrNew(size);
 if(p){used+=size;if(used>max_used)max_used=used;}
 return p;
}void
mem_free(void*p)
{unsigned long size=MemPtrSize(p);
 MemPtrFree(p);used-=size;
}unsigned long
mem_max_used(void){return max_used;}
unsigned long
mem_unfreed(void){return used;}
MemPtr
handle_lock(MemHandle h)
{num_locked++;if(num_locked>max_locked)max_locked=num_locked;
 return MemHandleLock(h);
}Err
handle_unlock(MemHandle h){num_locked--;return MemHandleUnlock(h);}
long
handles_lock_count(void){return num_locked;}
long
handles_max_lock_count(void){return max_locked;}
#endif
#define mem_db_type ('Cach')
enum local_enums{mem_handles_num=0x11,min_chunk_size=4};
enum debug_constants{facunde=1};
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
  draw_chars("unfreed handle");
 if(!db)return;shrink_records();DmCloseDatabase(db);
}struct mem_chunk
alloc_chunk(unsigned size)
{struct mem_chunk m;int i;m.d=-1;
 for(i=0;i<mem_handles_num&&busy_handles[i];i++);
 if(!db){draw_chars("no db");return m;}
 if(i==mem_handles_num){draw_chars("all busy");return m;}
 if(!(busy_handles[i]=DmResizeRecord(db,i,size)))
 {draw_chars("can't resize  ");return m;}
 m.d=i;m.p=MemHandleLock(busy_handles[i]);return m; 
}
/*int
write_chunk(struct mem_chunk m,unsigned offset,const void*src,unsigned size)
{return DmWrite(m.p,offset,src,size);}*/
const char*
lock_chunk(struct mem_chunk m)
{MemHandleUnlock(busy_handles[m.d]);DmReleaseRecord(db,m.d,0);
 busy_handles[m.d]=DmQueryRecord(db,m.d);if(!busy_handles[m.d])return 0;
 return MemHandleLock(busy_handles[m.d]);
}void
free_chunk(struct mem_chunk p)
{mem_chunk_descriptor d=p.d;
 if(d>=mem_handles_num||d<0)return;MemHandleUnlock(busy_handles[d]);
 busy_handles[d]=0;DmResizeRecord(db,d,min_chunk_size);
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
