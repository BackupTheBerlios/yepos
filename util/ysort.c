/*ysort: yepos database pre-sort program*/
#ifdef __STDC__
#include<stdlib.h>
#else
#define const
#endif
#if NO_MALLOC_DECL
extern char*malloc();
#endif
#if NO_REALLOC_DECL
extern char*realloc();
#endif
#include<stdio.h>
#include<string.h>
#include"dynamic_buffer.h"
enum local_constants
{bits_per_byte=8,sort_table_length=1<<bits_per_byte,
 sort_table_mask=(1<<bits_per_byte)-1
};
static struct dynamic_buffer comment,sort_order,current_line;
static void
free_buffers()
{free_buffer(&comment);free_buffer(&sort_order);
 free_buffer(&current_line);
}
static FILE*input,*output;
static void
close_files(){if(input)fclose(input);if(output)fclose(output);}
static int
read_line(b)struct dynamic_buffer*b;
{int c;if(!input)return!0;clear_buffer(b);
 while(1)
 {c=getc(input);if(c=='\n'||c==EOF)break;
  if(append_char(b,c))return!0;
 }return 0;
}
static int
read_preamble()
{struct dynamic_buffer*buf;
 while(!feof(input))
 {if(read_line(&current_line))return!0;
  if(!current_line.used)return 0;
  if(current_line.s[0]!='#'&&current_line.s[0]!=' ')return 0;
  buf=('#'==current_line.s[0])?&comment:&sort_order;
  if(append_chars(buf,current_line.s,current_line.used))
   return!0;
  if(append_char(buf,'\n'))return!0;
 }return 0;
}
static unsigned char sort_table[sort_table_length];
static int
assign_sort_table()
{static const char default_order[]=
  " aA bB cC dD eE fF gG hH iI jJ kK lL mM\n nN oO pP qQ rR sS tT uU vV wW xX yY zZ";
 unsigned long i,n=sort_order.used;int assigned=0;
 const char*s=sort_order.s;
 if(!n){n=strlen(default_order);s=default_order;}
 for(i=0;i<n;i++)
 {while((!i||s[i-1]=='\n')&&i+1<n&&s[i]==' '&&s[i+1]==' ')
   while(++i<n&&s[i-1]!='\n');
  if(s[i]==' '){assigned++;while(s[++i]==' '&&i<n);}
  if(s[i]=='\n'&&i&&s[i-1]==' ')assigned--;
  if(s[i]!=' '&&s[i]!='\n')
   sort_table[((unsigned char)(s[i]))&sort_table_mask]=assigned;
 }
 for(i=1;i<sort_table_length;i++)if(sort_table[i])
  sort_table[i]+=sort_table_length-assigned-1;
 assigned=0;
 for(i=1;i<sort_table_length;i++)if(!sort_table[i])
  sort_table[i]=++assigned;
 return 0;
}
struct title{char*title,*notes,*body;};
static struct title*titles;
unsigned long titles_num,titles_allocated,titles_used;
static int
read_titles()
{struct title*t;titles_allocated=289;
 titles=(struct title*)malloc(titles_allocated*sizeof*titles);
 memset(titles,0,titles_allocated*sizeof*titles);
 do
 {unsigned long n=titles_used;
  if(!current_line.used)
  {fprintf(stderr,"empty article title detected\n");return!0;}
  if(n>=titles_allocated)
  {unsigned long ta=titles_allocated;
   titles_allocated+=titles_allocated/4;
   titles=(struct title*)realloc(titles,titles_allocated*sizeof*titles);
   memset(titles+n,0,(titles_allocated-ta)*sizeof*titles);
   if(!titles)return!0;
  }titles_used++;
  t=titles+n;t->title=malloc(current_line.used+1);if(!t->title)return!0;
  memcpy(t->title,current_line.s,current_line.used);
  t->title[current_line.used]=0;
  if(read_line(&current_line))return!0;
  t->notes=malloc(current_line.used+1);if(!t->notes)return!0;
  if(current_line.used)memcpy(t->notes,current_line.s,current_line.used);
  t->notes[current_line.used]=0;
  if(read_line(&current_line))return!0;
  t->body=malloc(current_line.used+1);if(!t->body)return!0;
  if(current_line.used)memcpy(t->body,current_line.s,current_line.used);
  t->body[current_line.used]=0;
  if(read_line(&current_line))return!0;
 }while(!feof(input));
 return 0;
}
static int
sort_weight(c)char c;
{return sort_table[((unsigned char)c)&sort_table_mask];}
static int
compare_titles(a_,b_)const void*a_;const void*b_;
{const struct title*at=(const struct title*)a_,
  *bt=(const struct title*)b_;
 const char*a=at->title,*b=bt->title;
 for(;*a&&*b;a++,b++)
 {if(sort_weight(*a)>sort_weight(*b))return 1;
  if(sort_weight(*a)<sort_weight(*b))return-1;
 }
 if(sort_weight(*a)>sort_weight(*b))return 1;
 if(sort_weight(*a)<sort_weight(*b))return-1;
 return 0;
}
static void
sort_titles()
{if(titles_used<2)return;
 qsort(titles,titles_used,sizeof*titles,compare_titles);
}
static void
output_title(t)struct title*t;
{fprintf(output,"%s\n",t->title);
 fprintf(output,"%s\n",t->notes?t->notes:"");
 fprintf(output,"%s\n",t->body?t->body:"");
}
static void
free_titles()
{unsigned long i;
 for(i=0;i<titles_used;i++)
 {if(titles[i].title)free(titles[i].title);
  if(titles[i].notes)free(titles[i].notes);
  if(titles[i].body)free(titles[i].body);
 }if(titles)free(titles);
}
#ifndef __DATE__
#define __DATE__ "[some day]"
#endif
static void
usage()
{static const char THYNAME[]="ysort";
 printf("%s 0.0 (built %s): yepos input pre-sort program\n",
  THYNAME,__DATE__);
 printf("Copyright (C) 2009 Ineiev<ineiev@users.berlios.de>, super V 93\n");
 printf("%s comes with NO WARRANTY, to the extent permitted by law.\n",
  THYNAME);
 printf("You may redistribute copies of %s\n",THYNAME);
 printf("under the terms of the GNU GPL v3+\n");
 printf("Usage: %s unsorted.txt sorted.txt\n",THYNAME);
}
static void
output_buf(b,f)struct dynamic_buffer*b;FILE*f;
{unsigned long i;
 for(i=0;i<b->used;i++)putc(b->s[i],f);
}
static void
output_comment(){output_buf(&comment,output);}
static void
output_sort_order(){output_buf(&sort_order,output);}
static void
output_titles()
{unsigned long i;for(i=0;i<titles_used;i++)output_title(titles+i);}
void
close_all(){free_buffers();close_files();free_titles();}
int
process_all(argc,argv)int argc;char**argv;
{usage();if(argc<3)return!0;
 printf("\ninput: `%s'; output: `%s'\n",argv[1],argv[2]);
 input=fopen(argv[1],"rt");
 if(!input)
 {fprintf(stderr,"can't open file `%s' for input\n",argv[1]);
  return!0;
 }
 output=fopen(argv[2],"wt");
 if(!output)
 {fprintf(stderr,"can't open file `%s' for output\n",argv[2]);
  close_all();return!0;
 }
 if(read_preamble())return!0;
 printf("comments block: <<EOC\n");output_buf(&comment,stdout);
 printf("EOC\nsort order block: <<EOO\n");output_buf(&sort_order,stdout);
 printf("EOO\n");if(read_titles())return!0;
 printf("%lu articles parsed\n",titles_used);
 if(assign_sort_table())return!0;
 printf("sorting articles... ");fflush(stdout);
 sort_titles();printf("done\n");
 printf("writing output... ");fflush(stdout);
 output_comment();output_sort_order();output_titles();
 printf("done\n");
 return 0;
}
int
main(argc,argv)int argc;char**argv;
{int ret=process_all(argc,argv);close_all();return ret;}
/*Copyright (C) 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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

