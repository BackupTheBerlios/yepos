/*yepos the PalmOS dictionary: memory handling
Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93

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
#ifdef COUNT_MEMORY
void*mem_new(unsigned long size);void mem_free(void*);
unsigned long mem_unfreed(void);unsigned long mem_max_used(void);
MemPtr handle_lock(MemHandle h);Err handle_unlock(MemHandle h);
long handles_lock_count(void);long handles_max_lock_count(void);
#else
#define mem_new MemPtrNew
#define mem_free MemPtrFree
#define mem_max_used() (0ul)
#define mem_unfreed() (0ul)
#define handle_lock(h) MemHandleLock(h)
#define handle_unlock(h) MemHandleUnlock(h)
#define handles_lock_count() (0l)
#define handles_max_lock_count() (0l)
#endif
typedef void*locked_pointer;
typedef int mem_chunk_descriptor;
struct mem_chunk{mem_chunk_descriptor d;locked_pointer p;};
struct mem_chunk alloc_chunk(unsigned size);
#define write_chunk(chunk,offset,src,size) DmWrite(chunk.p,offset,src,size)
const char*lock_chunk(struct mem_chunk);
void free_chunk(struct mem_chunk);
static inline void
destruct_chunk(struct mem_chunk*m){if(m->d<0)return;free_chunk(*m);m->d=-1;}
int init_memory(void);void close_memory(void);
