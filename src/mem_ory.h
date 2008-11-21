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
enum invalid_mem_chunk_descriptors
{invalid_chunk_descriptor=-1,no_chunk_db=-2,
 all_descriptors_busy=-3,chunk_resize_failed=-4,
 handle_lock_failed=-5
};
typedef void*locked_pointer;
typedef int mem_chunk_descriptor;
struct mem_chunk{mem_chunk_descriptor d;locked_pointer p;};
struct mem_chunk alloc_chunk(unsigned size);
static inline int
write_chunk(struct mem_chunk m,unsigned offset,const void*src,unsigned size)
{return DmWrite(m.p,offset,src,size);}
const char*lock_chunk(struct mem_chunk);
void free_chunk(struct mem_chunk);
static inline void
destruct_chunk(struct mem_chunk*m){if(m->d<0)return;free_chunk(*m);m->d=-1;}
int init_memory(void);void close_memory(void);
