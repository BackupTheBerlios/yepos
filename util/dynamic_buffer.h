/* simple dynamically allocated buffer */
struct dynamic_buffer{char*s;unsigned long allocated,used;};
static int
append_chars(struct dynamic_buffer*b,const char*s,int n)
{if(!n)return 0;
 if(!b->allocated)
 {b->s=malloc(n);if(!b->s)return!0;b->allocated=n;}
 else if(b->allocated<b->used+n)
 {while(b->allocated<b->used+n)
  {b->allocated+=b->allocated/4+289;}
  b->s=realloc(b->s,b->allocated);if(!b->s)return!0;
 }memcpy(b->s+b->used,s,n);b->used+=n;return 0;
}static int
append_char(struct dynamic_buffer*b,int c)
{char _=c;return append_chars(b,&_,1);}
static void
clear_buffer(struct dynamic_buffer*b){b->used=0;}
static void
free_buffer(struct dynamic_buffer*b)
{if(b->allocated){free(b->s);b->allocated=0;}}
static void
show_buff(const struct dynamic_buffer*b)
{int i;putc('`',stderr);
 for(i=0;i<b->used;i++)putc(b->s[i],stderr);
 fprintf(stderr,"'\n");
}
void
dynamic_buffer_suppress_gcc_warnings_on_unused_functions(void)
{(void)append_char;(void)append_char;(void)clear_buffer;
 (void)free_buffer;(void)show_buff;
}/*Copyright (C) 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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
