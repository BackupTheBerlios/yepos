/* convert dictunformat(1) output into yeposc input format
 Usage:
  dictunformat serb2croat.index<serb2croat.dict | dict2y>serb2croat.txt */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"dynamic_buffer.h"
#define DEBUG 0
static void
output_buffer(const struct dynamic_buffer*b,char prefix)
{int i;if(prefix)putchar(prefix);
 if(DEBUG){fprintf(stderr,"output buffer ");show_buff(b);}
 for(i=0;i<b->used;i++)
 {putchar(b->s[i]);if(b->s[i]=='\n'&&prefix)putchar(prefix);}putchar('\n');
}
static struct dynamic_buffer current_line,
 current_head,current_title,current_body,
 database_info,database_short,database_url;
void
free_buffers(void)
{free_buffer(&current_line);free_buffer(&current_head);
 free_buffer(&current_title);free_buffer(&current_body);
 free_buffer(&database_info);free_buffer(&database_short);
 free_buffer(&database_url);
}
static int
read_line(struct dynamic_buffer*b)
{int c;clear_buffer(b);
 while(1)
 {c=getchar();if(c=='\n'||c==EOF)break;
  if(append_char(b,c))return!0;
 }return 0;
}
static void
output_header(void)
{if(database_short.used)
 {output_buffer(&database_short,'#');
  if(database_url.used||database_info.used)printf("#\n");
 }
 if(database_url.used)
 {output_buffer(&database_url,'#');
  if(database_info.used)printf("#\n");
 }
 if(database_info.used)output_buffer(&database_info,'#');
}static int
read_head(void)
{while(1)
 {if(read_line(&current_head))return!0;
  if(DEBUG){fprintf(stderr,"read_head: ");show_buff(&current_head);}
  if(current_head.used&&current_head.s[0]!=' '
     &&strncmp("_____",current_head.s,5))break;
  if(DEBUG&&current_head.used)
  {fprintf(stderr,"note: skipping %lu chars ",current_head.used);
   show_buff(&current_head);
  }
  if(feof(stdin))return!0;
 }return 0;
}
static const char dbsig[]="00-database-",short_sig[]="00-database-short",
 info_sig[]="00-database-info",url_sig[]="00-database-url";
static int header_gone;
static int
match_header(struct dynamic_buffer*b,const char*sig)
{int n;if(header_gone)return 0;n=strlen(sig);
 if(DEBUG)
 {fprintf(stderr,"matching against `%s' head ",sig);
  show_buff(&current_head);
 }
 if(current_head.used!=n||strncmp(current_head.s,sig,n))return 0;
 if(DEBUG)fprintf(stderr,"head matched\n");
 clear_buffer(b);return append_chars(b,current_body.s,current_body.used);
}
static void
output_article(void)
{output_buffer(&current_head,0);
 {char*s=current_title.s;unsigned long n=current_title.used;
  if(current_head.used<=current_title.used
   &&!strncmp(current_head.s,current_title.s,current_head.used))
  {current_title.used-=current_head.used;current_title.s+=current_head.used;}
  output_buffer(&current_title,0);
  current_title.s=s;current_title.used=n;
 }output_buffer(&current_body,0);
}
static int
add_body_line(const struct dynamic_buffer*b)
{int i;
 for(i=0;i<b->used&&b->s[i]==' ';i++);
 return append_chars(&current_body,b->s+i,b->used-i);
}
static int
read_body(void)
{if(DEBUG)
 {fprintf(stderr,"current title ");show_buff(&current_title);}
 while(!(read_line(&current_line)||feof(stdin)))
 {if(DEBUG){fprintf(stderr,"body line: ");show_buff(&current_line);}
  if(!current_line.used)
  {if(header_gone){if(current_body.used)output_article();return 0;}
   continue;
  }
  if(current_body.used&&current_line.s[0]!=' ')
  {if(DEBUG){fprintf(stderr,"read body ");show_buff(&current_body);}
   if(match_header(&database_short,short_sig))return!0;
   if(match_header(&database_info,info_sig))return!0;
   if(match_header(&database_url,url_sig))return!0;
   if(header_gone)output_article();return 0;
  }
  if(current_body.used)
  {char s[0x11];if(header_gone)sprintf(s," ||");else sprintf(s,"\n");
   if(append_chars(&current_body,s,strlen(s)))return!0;
  }
  if(add_body_line(&current_line))return!0;
 }if(header_gone&&current_body.used)output_article();return!0;
}
static int
read_title(void)
{int n=strlen(dbsig);clear_buffer(&current_title);clear_buffer(&current_body);
 if(DEBUG){fprintf(stderr,"current head ");show_buff(&current_head);}
 if(n>current_head.used)n=current_head.used;
 if(strncmp(dbsig,current_head.s,n)&&!header_gone)
 {output_header();header_gone=!0;
  if(DEBUG)fprintf(stderr,"header gone\n");
 }
 if(read_line(&current_title))return!0;
 if(current_title.used&&current_title.s[0]==' ')
 {if(DEBUG){fprintf(stderr,"mistitle ");show_buff(&current_title);}
  add_body_line(&current_title);clear_buffer(&current_title);
 }return 0;
}
static int
process_next_article(void)
{return!(read_head()||read_title()||read_body());}
static void
usage(void)
{static const char THYNAME[]="dict2y";
 printf("%s 0.1 (built "__DATE__"): DICT to yeposc input format converter\n"
 "Copyright (C) 2009 Ineiev<ineiev@users.berlios.de>, super V 93\n"
 "%s comes with NO WARRANTY, to the extent permitted by law.\n"
 "You may redistribute copies of %s under the terms of the GNU GPL v3+\n"
 "Usage: dictunformat serb2croat.index <serb2croat.dict \\\n"
 "       | %s >serb2croat.txt\n"
 "  where\n"
 "   dictunformat is a program coming with dictd distribution;\n"
 "   serb2croat.index is DICT database index file name;\n"
 "   serb2croat.dict is DICT uncompressed database file name\n"
 "    (typical uncompression command is\n"
 "     `gzip -dc <serb2croat.dict.dz >serb2croat.dict')\n",
 THYNAME,THYNAME,THYNAME,THYNAME);
}
int
main(int argc,char**argv)
{if(argc>1){usage();return!0;}
 while(process_next_article()&&!feof(stdin));free_buffers();return 0;
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
