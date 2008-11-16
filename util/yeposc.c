/* yepos database compiler
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
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<zlib.h>
#include"../include/signs.h"
static const char THYNAME[]="yeposc",creator[]=CREATOR_STRING,
 db_type[]=DB_TYPE_STRING,default_db_name[]="unnamed.yepos";
enum local_constants
{default_record_length=4096,
 default_initial_index_length=16384,
 max_index_length=32766,
 default_max_title=289,
 palm_ushort_size=2,palm_ulong_size=4,
 number_of_articles_size=palm_ushort_size,
 feature_bits_size=palm_ushort_size,
 arity_number_size=palm_ushort_size,
 item_number_size=palm_ushort_size,
 volume_number_size=palm_ushort_size,
 record_number_size=palm_ushort_size,
 pointer_to_item_size=palm_ushort_size,
 content_constant_header_size=number_of_articles_size,
 index_constant_header_size=arity_number_size+item_number_size,
 pointer_to_article_size=palm_ushort_size,
 content_increment_size=pointer_to_article_size,
 index_increment_size=volume_number_size+record_number_size
  +pointer_to_item_size,
 max_arity=0x33,
 VERSION=0,bits_per_byte=8,byte_mask=(1<<bits_per_byte)-1,
 db_name_length=32,
 creator_length=palm_ulong_size,db_type_length=palm_ulong_size
};
static unsigned char default_translit_table[1<<bits_per_byte],
 *translit_table=default_translit_table;
static unsigned features;
static char db_name[db_name_length];
static const unsigned long max_records_per_db=65534;
static unsigned long out_cnt_minor,out_cnt_maior;
static int verbous;
static int
fputc_counted(int c,FILE*f)
{int r=putc(c,f);if(!++out_cnt_minor)++out_cnt_maior;return r;}
static void
write_lu(FILE*f,unsigned long x)
{fputc_counted((x>>(bits_per_byte*3))&byte_mask,f);
 fputc_counted((x>>(bits_per_byte*2))&byte_mask,f);
 fputc_counted((x>>bits_per_byte)&byte_mask,f);
 fputc_counted(x&byte_mask,f);
}static void
write_hu(FILE*f,unsigned short x)
{fputc_counted((x>>bits_per_byte)&byte_mask,f);
 fputc_counted(x&byte_mask,f);
}static void
put_hu(char*s,unsigned short x)
{*s=(x>>bits_per_byte)&byte_mask;s[1]=x&byte_mask;}
static void
write_name(FILE*f)
{int i;const char*s=db_name;
 for(i=0;s[i];fputc_counted(s[i++],f));
 while(i++<db_name_length)fputc_counted(0,f);
}
static void
write_attributes(FILE*f){write_hu(f,0);}
static void
write_version(FILE*f){write_hu(f,VERSION);}
static void
write_creation_date(FILE*f){write_lu(f,17);}
static void
write_modification_date(FILE*f){write_lu(f,17);}
static void
write_last_backup_date(FILE*f){write_lu(f,17);}
static void
write_modification_number(FILE*f){write_lu(f,0);}
static void
write_app_info_id(FILE*f){write_lu(f,0);}
static void
write_sort_info_id(FILE*f){write_lu(f,0);}
static void
write_type(FILE*f)
{int i;for(i=0;i<db_type_length;i++)fputc_counted(db_type[i],f);}
static void
write_creator(FILE*f)
{int i;for(i=0;i<creator_length;i++)fputc_counted(creator[i],f);}
static void
write_unique_id_seed(FILE*f){write_lu(f,17);}
static void
write_unique_id(FILE*f)
{static unsigned long id;unsigned long d=++id;
 fputc_counted(d&byte_mask,f);d>>=bits_per_byte;
 fputc_counted(d&byte_mask,f);d>>=bits_per_byte;
 fputc_counted(d&byte_mask,f);
}
static unsigned initial_index_length=default_initial_index_length;
static void
usage(void)
{printf("%s 0.1 (built "__DATE__"): yepos database compiler\n"
 "Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93\n"
 "%s comes with NO WARRANTY, to the extent permitted by law.\n"
 "You may redistribute copies of %s under the terms of the GNU GPL v3+\n"
 "Usage: %s -i in_file_name -o out_file_name\n"
 " [-r record_size] [-t max_title] [-f features] [-x index_record_size]\n",
 THYNAME,THYNAME,THYNAME,THYNAME);
}
static const char*txt_name,*out_name;
static char*record_buf,*compressed_record_buf,*title_buf,
 *upcased_title_buf,*prev_title_buf,
 record_tail[65536],comment_string[16384];
static FILE*content_file;
static const char content_file_name[]="yepos.0";
static void
index_file_name(char*s,int i){sprintf(s,"yeposidx.%i",i);}
/*File structures are arranged so that PalmOS executable
can reference to the values directly from the locked chunk; this
means that all 2-byte fields are even-aligned within the record */
/* content record structure:
 (PalmOS unsigned short) number of articles
[repeat number of articles times]
{ pointers within the record:
 (PalmOS unsigned short) pointer to the title
}
[repeat number of articles times]
{title\0grammatical notes\0body\0}
if the record is compressed, it's uncompressed size
is placed in (PalmOS unsigned short) before the compressed chunk */
/* primary, secondary, tertiary and so on indices have the next structure:
 (PalmOS unsigned short) -arity number (1 is for primary)
 (PalmOS unsigned short) number lower level items pointed by the index
[repeat the next number of items times]
{
 (PalmOS unsigned short) volume number of the item
 (PalmOS unsigned short) record number of the item 
 (PalmOS unsigned short) pointer to the first title within the index structure
}
[repeat the next number of items times]
{ASCIIZ first article title}
these records are not compressed; their sizes are limited by PalmOS record
size limit only */
/* the first record of the volume has the next structure:
(PalmOS unsigned short) feature bits:
 (LSB) 0 - compression enabled
       1 - translit table included (uninplemented yet)
 (MSB)
(PalmOS unsigned short) maximum uncompressed content record length
(PalmOS unsigned short) maximum -arity number
(PalmOS unsigned short) number of volumes
(PalmOS unsigned short) current volume number
[repeat maximum -arity number+1 times]
{
 (PalmOS unsigned short) number of the first n-ary record
  within the volume (0 if none)
}
the rest is the string describing the dictionary */
/* VFS (plain stream) dictionary begins with list of records:
 (PalmOS unsigned short) number of volumes in the file
[repeat number of volumes times]
{(PalmOS unsigned long)  next volume records list offset}
[repeat number of volumes times]
{
 (PalmOS unsigned short) number of records in the volume
 [repeat number of records+1 times]
 {(PalmOS unsigned long)  offset of the next record within the file}
} */
static unsigned record_length=(unsigned)default_record_length,
 compressed_record_length,max_title=(unsigned)default_max_title,
 record_tail_length;
static unsigned max_title_observed,max_article_observed;
static unsigned long articles,content_records,total_records;
static unsigned long byte_minor,byte_maior,line_minor,line_maior;
static struct current_index_pointer
{unsigned long records,buf_size,caput,items,max_items,overall_items;
 char*buf;FILE*content;
}current_indices[max_arity];
static unsigned
additional_index_stuff(unsigned items)
{return index_constant_header_size+items*index_increment_size;}
static int
init_indices(void)
{int i;struct current_index_pointer*ci;
 for(i=0,ci=current_indices;i<max_arity;i++,ci++)
 {char*p=malloc(initial_index_length);if(!p)return-1;
  ci->buf=p;ci->buf_size=initial_index_length;
  ci->max_items=ci->caput=ci->items=ci->overall_items=ci->records=0;
 }return 0;
}static void
close_indices(void)
{int i;
 for(i=0;i<max_arity;i++)
 {struct current_index_pointer*ci=current_indices+i;
  if(ci->buf)
  {free(ci->buf);ci->buf=0;ci->buf_size=0;}
  if(ci->content)
  {char s[0x33];fclose(ci->content);ci->content=0;
   index_file_name(s,i);remove(s);
  }
 }
 if(content_file)
 {fclose(content_file);content_file=0;remove(content_file_name);}
}
static FILE*
index_file(int i,const char*mode)
{char s[0x22];index_file_name(s,i);return fopen(s,mode);}
static int
redopen_indicium_contents(void)
{unsigned i;
 for(i=0;i<max_arity;i++)
 {struct current_index_pointer*ci=current_indices+i;
  if(ci->content)
  {fclose(ci->content);ci->content=index_file(i,"rb");
   if(!ci->content)
   {fprintf(stderr,"can't open file `%s'\n",content_file_name);
    return!0;
   }
  }
 }
 if(content_file)
 {fclose(content_file);
  if(!(content_file=fopen(content_file_name,"rb")))
  {fprintf(stderr,"can't open file `%s'\n",content_file_name);
   return!0;
  }
 }return 0;
}
static int
ungetc_counted(int c,FILE*f)
{int r=ungetc(c,f);if(!byte_minor--)--byte_maior;return r;}
static int
getc_counted(FILE*f)
{int c=getc(f);if(c!=EOF)if(!++byte_minor)++byte_maior;
 if(c=='\n')if(!++line_minor)++line_maior;return c;
}
static void
report_pos(void)
{fprintf(stderr,"error occured on ");
 if(byte_maior)fprintf(stderr,"0x%8.8lX%8.8lX",byte_maior,byte_minor);
 else fprintf(stderr,"%lu",byte_minor);
 fprintf(stderr," byte, ");
 if(line_maior)fprintf(stderr,"0x%8.8lX%8.8lX",line_maior,line_minor);
 else fprintf(stderr,"%lu",line_minor);
 fprintf(stderr, " line\n");
 fprintf(stderr,"article %li, content_record %li\n",
  articles,content_records);
}
static int
translit(int c)
{if(c>=0&&c<sizeof(default_translit_table))
  return translit_table[c];
 return c;
}
static int
read_title(FILE*f)
{int c,n;static int compare_previous;char*p;
 p=prev_title_buf;prev_title_buf=upcased_title_buf;
 upcased_title_buf=p;
 for(n=c=0;n<max_title;n++)
 {c=getc_counted(f);
  if(c==EOF)
  {upcased_title_buf[n]=title_buf[n]=0;if(!n)return 0;
   fprintf(stderr,"EOF while reading article title\n");
   fprintf(stderr,"previous characters were `%s'\n",title_buf);
   report_pos();return-1;
  }if(c=='\n')break;title_buf[n]=c;upcased_title_buf[n]=translit(c);
 }upcased_title_buf[n]=title_buf[n]=0;
 if(c!='\n')
 {fprintf(stderr,"title too long (more than %i bytes)\n",n);
  report_pos();return-3;
 }
 if(compare_previous)if(strcmp(prev_title_buf,upcased_title_buf)>0)
 {fprintf(stderr,"unsorted articles found: `%s'>`%s'\n",
   prev_title_buf,upcased_title_buf);
  report_pos();
 }compare_previous=!0;
 if(max_title_observed<n)max_title_observed=n;return n;
}static int
read_article(FILE*f)
{int c;unsigned n,size=sizeof(record_tail)-2;char*buf=record_tail;
 c=read_title(f);if(c<=0)return c*0x33;
 if((n=strlen(title_buf)+1)>size)
 {fprintf(stderr,"too long (%u) title `%s'\n",n,title_buf);
   report_pos();return-1;
 }strcpy(buf,title_buf);
 for(c=0;c!='\n'&&n<size;n++)
 {c=getc_counted(f);
  if(c==EOF)
  {buf[n]=0;
   fprintf(stderr,"EOF while reading grammatical marks\n");
   fprintf(stderr,"the title was `%s'\n",buf);
   report_pos();return-2;
  }buf[n]=c;
 }buf[n-1]=0;
 if(n>=size)
 {fprintf(stderr,"too long grammatical marks\n");
  fprintf(stderr,"the title was `%s'\n",buf);
  report_pos();return-3;
 }
 while(n<size)
 {c=getc_counted(f);
  if(c==EOF)
  {buf[n]=0;
   fprintf(stderr,"EOF while reading article body\n");
   fprintf(stderr,"the title was `%s'\n",buf);
   report_pos();return-4;
  }if(c=='\n')break;buf[n++]=c;
 }buf[n]=0;
 if(n>=size)
 {fprintf(stderr,"too long article body\n");
  fprintf(stderr,"the title was `%s'\n",buf);
  report_pos();return-5;
 }return n+1;
}
static unsigned
additional_stuff(unsigned articles)
{return content_constant_header_size+articles*content_increment_size;}
static void
print_indices_statistics(void)
{int i;total_records=0;
 for(i=0;i<max_arity;i++)
 {printf("%i-ary index: %lu records\n",
   i+1,current_indices[i].records+1);
  printf("%i-ary index: %lu items\n",
   i+1,current_indices[i].overall_items);
  if(!current_indices[i].records)
   current_indices[i].max_items=current_indices[i].items;
  printf("%i-ary index: %lu items maximum\n",
   i+1,current_indices[i].max_items);
  total_records+=current_indices[i].records+1;
  if(!current_indices[i].records)break;
 }total_records+=content_records+1+1;
 printf("total records:   %lu\n",total_records);
}
static unsigned long max_art_per_rec;
static void
print_statistics(void)
{printf("database name:  `%s'\n",db_name);
 printf("full comment:   `%s'\n",comment_string);
 print_indices_statistics();
 printf("content records: %lu\n",content_records+1);
 printf("articles:        %lu\n",articles);
 printf("articles/record: %lu maximum\n",max_art_per_rec);
 printf("maximum title:   %u bytes\n",max_title_observed);
 printf("maximum article: %u bytes\n",max_article_observed);
 printf("total lines:     %lu\n",line_minor);
 printf("total bytes:     %lu\n",byte_minor-1);
}
static void
write_idx_record(int ary)
{struct current_index_pointer*ci=current_indices+ary;
 FILE*f;unsigned i,n;const char*p;
 if(!ci->content)ci->content=index_file(ary,"wb");f=ci->content;
 write_hu(f,ary+1);write_hu(f,ci->items);
 n=additional_index_stuff(ci->items);
 for(i=0,p=ci->buf;i<ci->items;i++)
 {write_hu(f,0);write_hu(f,content_records);write_hu(f,n);
  while(*++p)++n;p++;n+=2;
 }for(i=0,p=ci->buf;i<ci->caput;i++,p++)fputc_counted(*p,f);
}static int
add_record_to_indices(int i)
{struct current_index_pointer*ci=current_indices+i;
 int n=strlen(title_buf)+1;if(i>=max_arity)return 0;
 ci->overall_items++;
 if(ci->caput+n+additional_index_stuff(ci->items+1)>=ci->buf_size)
 {ci->records++;write_idx_record(i);
  if(ci->items>ci->max_items)ci->max_items=ci->items;
  ci->caput=0;ci->items=0;
  if(ci->caput+n+additional_index_stuff(ci->items+1)>=ci->buf_size)
  {fprintf(stderr,"too long item\n");report_pos();return-1;}
 }if(!ci->items)add_record_to_indices(i+1);
 ci->items++;memmove(ci->buf+ci->caput,upcased_title_buf,n);
 ci->caput+=n;return 0;
}
unsigned short
get_hu(FILE*f)
{return (getc_counted(f)<<bits_per_byte)|getc_counted(f);}
unsigned long
get_ul(FILE*f)
{return((unsigned long)(getc_counted(f))<<(bits_per_byte*3))
  |((unsigned long)(getc_counted(f))<<(bits_per_byte*2))
  |((unsigned long)(getc_counted(f))<<bits_per_byte)
  |getc_counted(f);
}
static unsigned first_record_size;
unsigned
max_arity_present(void)
{unsigned i;
 for(i=0;i<max_arity;i++)
  if(!current_indices[i].records)break;
 return i+1;
}
static int
write_record_entries(FILE*f,unsigned long pos)
{int c;unsigned i,j,k,entries=0;FILE*in=content_file;
 redopen_indicium_contents();byte_minor=pos;byte_maior=0;
 if(verbous)printf("record entries %lu (%lu)\n",out_cnt_minor,byte_minor);
 {unsigned ma=max_arity_present();unsigned long pos0=byte_minor;
  write_lu(f,byte_minor);fputc_counted(0,f);write_unique_id(f);
  byte_minor+=feature_bits_size;byte_minor+=record_number_size;
  byte_minor+=arity_number_size;byte_minor+=volume_number_size*2;
  byte_minor+=(ma+1)*record_number_size;
  byte_minor+=strlen(comment_string);
  first_record_size=byte_minor-pos0;
  if(verbous)printf("record 0 length %lu",byte_minor-pos0);
  if(verbous)printf(" (%u): %lu\n",ma,out_cnt_minor);
  entries++;
 }
 for(i=0;i<content_records+1;i++)if(features&compression_bit)
 {unsigned lng;unsigned long m0=byte_minor,m1=byte_maior;
  write_lu(f,byte_minor);fputc_counted(0,f);write_unique_id(f);
  lng=get_hu(in);for(j=0;j<lng;j++)getc_counted(in);
  byte_minor=lng+m0;byte_maior=m1;if(byte_minor<m0)++byte_maior;
 }else
 {unsigned art,ptr;
  unsigned long pos0=byte_minor;
  write_lu(f,byte_minor);fputc_counted(0,f);write_unique_id(f);
  art=get_hu(in);
  for(j=0;j<art;j++)ptr=get_hu(in);
  for(j=0;j<art;j++)for(k=0;k<3;k++)while((c=getc_counted(in)));
  entries++;
  if(verbous)
   printf("content %u record entry: %lu:%lu (%lu))\n",
    i,pos0,byte_minor,out_cnt_minor);
 }
 for(i=0;i<max_arity;i++)
 {struct current_index_pointer*ci=current_indices+i;
  FILE*in=ci->content;unsigned ary,items,vol,rec,title;if(!in)break;
  while(1)
  {ary=get_hu(in);
   if(feof(in))break;
   write_lu(f,byte_minor-arity_number_size);fputc_counted(0,f);write_unique_id(f);
   if(ary-1!=i){fprintf(stderr,"arity does not match(%u/%u)\n",ary-1,i);return!0;}
   items=get_hu(in);
   for(j=0;j<items;j++)
   {vol=get_hu(in);
    if(vol){fprintf(stderr,"wrong volume %u\n",vol);return!0;}
    rec=get_hu(in);title=get_hu(in);
   }
   for(j=0;j<items;j++)
   {while((c=getc_counted(in)))if(c==EOF)
    {fprintf(stderr,"unexpected EOF\n");break;}
   }
   if(verbous)
   {printf("idx %u record entry: (%u items) %lu: ",
     i+1,items,byte_minor);
    printf("%lu (%lu)\n",byte_minor,out_cnt_minor);
    entries++;
   }
  }
 }if(verbous)printf("list entries %u\n",entries);
 return 0;
}static void
write_record_list(FILE*db)
{unsigned tr=total_records;
 write_lu(db,0);write_hu(db,tr);
 if(!total_records){write_hu(db,0);return;}
 write_record_entries(db,78+tr*8);
 if(verbous)printf("records begin %lu\n",out_cnt_minor);
}
static int
write_records(FILE*db)
{int c;unsigned cur_rec,i,j;
 redopen_indicium_contents();
 {unsigned ma=max_arity_present();unsigned long pos0=out_cnt_minor;
  write_hu(db,features);write_hu(db,record_length);
  write_hu(db,ma);write_hu(db,1);
  write_hu(db,0);write_hu(db,1);
  cur_rec=content_records+2;write_hu(db,cur_rec);
  if(verbous)
   printf(" arity %u; volumes %u; vol %u;content start %u;"
          "primary start %u\n",ma,1,0,1,cur_rec);
  for(i=0;i<ma-1;i++)
  {cur_rec+=current_indices[i].records+1;write_hu(db,cur_rec);}
  for(i=0;comment_string[i];i++)fputc_counted(comment_string[i],db);
  if(verbous)
   printf("record 0 %lu/ %lu(%u)\n",out_cnt_minor,
    out_cnt_minor-pos0,ma);
 }
 if(features&compression_bit)while(1)
 {unsigned lng=get_hu(content_file);if(feof(content_file))break;
  for(i=0;i<lng;i++)fputc_counted(getc(content_file),db);
 }else while(EOF!=(c=getc(content_file)))fputc_counted(c,db);
 if(verbous)printf("first index %lu\n",out_cnt_minor);
 cur_rec=1;
 for(i=0;i<max_arity;i++)
 {struct current_index_pointer*ci=current_indices+i;
  FILE*f=ci->content;unsigned ary,items,vol,rec,title;if(!f)break;
  if(verbous)printf("%u-ary index %lu\n",i+1,out_cnt_minor);
  while(1)
  {ary=get_hu(f);if(feof(f))break;write_hu(db,ary);
   if(ary-1!=i)
   {fprintf(stderr,"arity does not match(%u/%u)\n",ary-1,i);return!0;}
   items=get_hu(f);write_hu(db,items);
   if(verbous)printf("arity %u; items %u\n",ary,items);
   for(j=0;j<items;j++)
   {vol=get_hu(f);write_hu(db,vol);
    if(vol){fprintf(stderr,"wrong volume %u\n",vol);return!0;}
    rec=get_hu(f);write_hu(db,cur_rec++);
    title=get_hu(f);write_hu(db,title);
    if(verbous)
     printf("item %u; vol %u; title ptr %u\n",j,vol,title);
   }
   for(j=0;j<items;j++)
   {if(verbous)printf("title %u: `",j);
    while((c=getc_counted(f)))
    {if(c!=EOF)fputc_counted(c,db);else break;
     if(verbous)putchar(c);
    }if(verbous)printf("'\n");
    fputc_counted(0,db);
   }
  }
 }return 0;
}static void
write_header(FILE*db)
{write_name(db);write_attributes(db);write_version(db);
 write_creation_date(db);write_modification_date(db);
 write_last_backup_date(db);write_modification_number(db);
 write_app_info_id(db);write_sort_info_id(db);
 write_type(db);write_creator(db);
 write_unique_id_seed(db);write_record_list(db); 
}static int
write_database(FILE*db)
{int r;out_cnt_minor=out_cnt_maior=0;write_header(db);
 r=write_records(db);fclose(db);return r;
}static unsigned art_in_record;
static int
write_content_record(unsigned caput)
{FILE*f=content_file;unsigned j,n;int i;const char*p;
 n=art_in_record*pointer_to_article_size
   +content_constant_header_size;
 if(features&compression_bit)
 {char*dest=record_buf;unsigned m=n;int ret;
  for(i=caput-1;i>=0;i--)record_buf[n+i]=record_buf[i];
  put_hu(dest,art_in_record);dest+=palm_ushort_size;
  for(i=0,p=record_buf+n;i<art_in_record;i++)
  {put_hu(dest,n);dest+=palm_ushort_size;
   for(j=0;j<3;j++){while(*p){++p;++n;}p++;n++;}
  }caput+=m;
  {uLongf dest_len=compressed_record_length,source_len=caput;
   ret=compress2((Bytef*)compressed_record_buf,&dest_len,
     (const Bytef*)record_buf,source_len,9);
   if(ret!=Z_OK){fprintf(stderr,"compression failed\n");return!0;}
   write_hu(f,dest_len+palm_ushort_size);write_hu(f,source_len);
   for(i=0;i<dest_len;i++)fputc_counted(compressed_record_buf[i],f);
  }
 }else
 {write_hu(f,art_in_record);
  for(i=0,p=record_buf;i<art_in_record;i++)
  {write_hu(f,n);for(j=0;j<3;j++){while(*p){++p;++n;}p++;n++;}}
  for(i=0,p=record_buf;i<caput;i++,p++)fputc_counted(*p,f);
 }return 0;
}
static int
read_comments(FILE*f)
{int c,caput=0;
 while(1)
 {c=getc_counted(f);
  if(c!='#')
  {ungetc_counted(c,f);comment_string[caput-1]=0;
   for(c=0;comment_string[c]&&comment_string[c]!='\n'
    &&c<db_name_length;c++)
    db_name[c]=comment_string[c];
   if(!c)strcpy(db_name,default_db_name);
   return 0;
  }
  do
  {c=getc_counted(f);
   if(EOF==c)
   {fprintf(stderr,"unexpected EOF in comments\n");
    report_pos();return!0;
   }
   if(!c)
   {fprintf(stderr,"NULL character in comments\n");
    report_pos();return!0;
   }comment_string[caput++]=c;
  }while(c!='\n');
 }return!0;
}
static int
count_content_records(void)
{FILE*f;int n;unsigned caput=0;
 f=fopen(txt_name,"rt");
 if(!f)
 {fprintf(stderr,"can't open file `%s' for reading\n",txt_name);
  return-1;
 }
 if(read_comments(f))return-1;
 while((n=read_article(f))>0)
 {articles++;if(max_article_observed<n)max_article_observed=n;
  if(n+caput+additional_stuff(art_in_record+1)>=record_length)
  {if(art_in_record>max_art_per_rec)max_art_per_rec=art_in_record;
   write_content_record(caput);
   content_records++;caput=0;art_in_record=0;
   if(n+caput+additional_stuff(art_in_record+1)>=record_length)
   {fprintf(stderr,"too long article\n");report_pos();return-1;}
  }if(!art_in_record)add_record_to_indices(0);
  art_in_record++;memmove(record_buf+caput,record_tail,n);caput+=n;
 }
 if(art_in_record)
 {int i;write_content_record(caput);
  if(art_in_record>max_art_per_rec)max_art_per_rec=art_in_record;
  for(i=0;i<max_arity;i++)
  {write_idx_record(i);if(!current_indices[i].records)break;}
 }
 if(n)
 {fprintf(stderr,"an error occured (%i)\n",n);
  record_buf[sizeof(record_buf)-1]=0;
  fprintf(stderr,"record content is`\n%s\n'\n",record_buf);
  record_tail[record_tail_length]=0;
  fprintf(stderr,"record tail is`\n%s\n'\n",record_tail);
  fprintf(stderr,"last record title is`\n%s\n'\n",title_buf);
 }fclose(f);print_statistics();return 0;
}static void
close_all(void)
{close_indices();
 if(compressed_record_buf)
 {free(compressed_record_buf);compressed_record_buf=0;}
 if(record_buf){free(record_buf);record_buf=0;}
 if(title_buf){free(title_buf);title_buf=0;}
 if(prev_title_buf){free(prev_title_buf);prev_title_buf=0;}
 if(upcased_title_buf)
 {free(upcased_title_buf);upcased_title_buf=0;}
}static void
init_translit_table(void)
{int i;
 for(i=0;i<sizeof default_translit_table;i++)
  default_translit_table[i]=tolower(i);
}
static int
parse_args(int argc,char**argv)
{char*s;argc--;argv++;
 while(argc)
 {if(**argv!='-')
  {fprintf(stderr,"wrong argument `%s'\n",*argv);
   usage();return!0;
  }
  switch(argv[0][1])
  {case'i':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;txt_name=s;argc--;argv++;
    break;
   case'o':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;out_name=s;argc--;argv++;
    break;
   case't':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;argc--;argv++;
    if(1!=sscanf(s,"%u",&max_title))
    {fprintf(stderr,"wrong maximum title length `%s'\n",s);
     return!0;
    }break;
   case'r':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;argc--;argv++;
    if(1!=sscanf(s,"%u",&record_length))
    {fprintf(stderr,"wrong content record length `%s'\n",s);
     return!0;
    }break;
   case'x':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;argc--;argv++;
    if(1!=sscanf(s,"%u",&initial_index_length))
    {fprintf(stderr,"wrong index record length `%s'\n",s);
     return!0;
    }break;
   case'f':
    if(!argv[0][2]){s=argv[1];argv++;argc--;}
    else s=argv[0]+2;argc--;argv++;
    if(1!=sscanf(s,"%X",&features))
    {fprintf(stderr,"wrong features code `%s'\n",s);
     return!0;
    }break;
   default:
    fprintf(stderr,"unknown option `%s'\n",*argv);return!0;
  }
 }return 0;
}
int
main(int argc,char**argv)
{int ret=0;
 if(parse_args(argc,argv)){usage();return!0;}
 if(!txt_name){fprintf(stderr,"no input file defined\n");return!0;}
 if(!out_name)
 {fprintf(stderr,"no output file defined\n");return!0;}
 features&=implemented_features_bits;
 printf("source file:          `%s'\n",txt_name);
 printf("destination file:     `%s'\n",out_name);
 printf("content record length: %u\n",record_length);
 printf("maximum title length:  %u\n",max_title);
 printf("index record length:   %u\n",initial_index_length);
 printf("features:              0x%4.4X",features);
 if(features)
 {int vex=0;unsigned f_=features;printf(" (");
  if(compression_bit&f_)
  {vex=!0;printf("compressed");f_&=~compression_bit;}
  if(upcoding_table_bit&f_)
  {if(vex)printf(" ");printf("upcoding");f_&=~upcoding_table_bit;}
  if(f_){if(vex)printf(" ");printf("[unknown]");}
  printf(")");
 }printf("\n");
 record_buf=malloc(record_length);
 if(!record_buf)
 {printf("can't allocate %u chars for record buffer\n",
   record_length);
  ret=3;goto exit;
 }
 if(features&compression_bit)
 {compressed_record_length=record_length*1.01+12;
  compressed_record_buf=malloc(compressed_record_length);
  if(!compressed_record_buf)
  {printf("can't allocate %u chars for record buffer\n",
    compressed_record_length);
   ret=3;goto exit;
  }
 }title_buf=malloc(max_title+1);
 if(!title_buf)
 {printf("can't allocate %u chars for title buffer\n",max_title+1);
  ret=3;goto exit;
 }upcased_title_buf=malloc(max_title+1);
 if(!upcased_title_buf)
 {printf("can't allocate %u chars for title buffer\n",max_title+1);
  ret=3;goto exit;
 }prev_title_buf=malloc(max_title+1);
 if(!prev_title_buf)
 {printf("can't allocate %u chars for title buffer\n",max_title+1);
  ret=3;goto exit;
 }
 init_translit_table();
 if(init_indices())goto exit;
 if(!(content_file=fopen(content_file_name,"wb")))
 {fprintf(stderr,"can't open file `%s'\n",content_file_name);}
 if(count_content_records())goto exit;
 if(total_records>max_records_per_db)
 {fprintf(stderr,"multiple volumes are not supported yet\n");
  goto exit;
 }
 {FILE*f=fopen(out_name,"wb");
  write_database(f);
 }
exit:close_all();return ret;
}
