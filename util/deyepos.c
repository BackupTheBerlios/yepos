/* yepos database compiler
Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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
#include<zlib.h>
#include"../include/signs.h"
static const char THYNAME[]="deyepos";
enum local_constants
{bits_per_byte=8,byte_mask=(1<<bits_per_byte)-1,
 db_name_length=32,creator_length=4,db_type_length=4,
 palm_ushort_size=2
};
static void
usage(void)
{printf("%s 0.1 (built "__DATE__"): yepos database extractor\n"
 "Copyright (C) 2008 Ineiev<ineiev@users.berlios.de>, super V 93\n"
 "%s comes with NO WARRANTY, to the extent permitted by law.\n"
 "You may redistribute copies of %s under the terms of the GNU GPL v3+\n"
 "Usage: %s in_file_name out_file_name\n",
 THYNAME,THYNAME,THYNAME,THYNAME);
}
unsigned long byte_minor,byte_maior;
static int
getc_counted(FILE*f)
{int c=getc(f);if(c!=EOF)if(!++byte_minor)++byte_maior;return c;}
static int
check_fopen(FILE**f,const char*n,const char*m)
{*f=fopen(n,m);if(*f)return 0;fprintf(stderr,"can't open %s\n",n);return!0;}
static unsigned short
get_hu(FILE*f)
{return (getc_counted(f)<<bits_per_byte)|getc_counted(f);}
static unsigned short
sget_hu(char*s)
{return ((((unsigned)s[0])&byte_mask)<<bits_per_byte)
  |(((unsigned)s[1])&byte_mask);
}
static unsigned long
get_lu(FILE*f)
{return((unsigned long)(getc_counted(f))<<(bits_per_byte*3))
  |((unsigned long)(getc_counted(f))<<(bits_per_byte*2))
  |((unsigned long)(getc_counted(f))<<bits_per_byte)
  |getc_counted(f);
}unsigned short rec_num;unsigned char*rec_list=0;
void
read_header(FILE*f)
{unsigned long x=!0;int n;printf("Name: \"");
 for(n=0;n<db_name_length;n++)
 {int c=getc_counted(f);if(!c&&x)printf("\"( ");if(x)x=c;
  if(x)putchar(x);else printf("%2.2X ",c);
 }putchar(x?'"':')');putchar('\n');
 printf("Attributes: 0x%4.4X; ",get_hu(f));printf("Version: %hu\n",get_hu(f));
 printf("Dates: Created: %lu; ",get_lu(f));printf("Modified: %lu; ",get_lu(f));
 printf("Backup: %lu\n",get_lu(f));
 printf("Modification Number: %lu\n",get_lu(f));
 printf("AppInfoID: %lu; ",get_lu(f));printf("SortInfoID: %lu\n",get_lu(f));
 x=get_lu(f);
 printf("Type: \"%c%c%c%c\"; ",
  (char)(x>>24),(char)(x>>16),(char)(x>>8),(char)x);
 x=get_lu(f);
 printf("Creator (application ID): \"%c%c%c%c\"\n",
  (char)(x>>24),(char)(x>>16),(char)(x>>8),(char)x);
 printf("UniqueIDSeed: %lu; ",get_lu(f));
 printf("NextRecordListID: %lu",x=get_lu(f));
 if(x)printf(" (multiple record lists are not supported)");
 printf("\nNumber of Records: %hu\n",rec_num=get_hu(f));
 if(feof(f)){fprintf(stderr,"end of file while reading header\n");rec_num=0;}
}int
get_rec_list(FILE*db)
{unsigned long i;
 if(!(rec_list=malloc(rec_num*8)))
 {fprintf(stderr,"can't allocate record list (%i records) in memory\n",rec_num);
  rec_num=0;return!0;
 }for(i=0;i<rec_num*8&&!feof(db);rec_list[i++]=getc_counted(db));
 if(feof(db))fprintf(stderr,"end of file while reading record list\n");
 return feof(db);
}unsigned long
get_rec_pos(unsigned short i)
{return((unsigned long)(rec_list[8*i])<<24)|
       ((unsigned long)(rec_list[8*i+1])<<16)|
       ((unsigned long)(rec_list[8*i+2])<<8)|rec_list[8*i+3];
}static int verbous=0;
int
write_list(FILE*db,char*f)
{int c;unsigned i,content_records;unsigned long pos,pos0;FILE*txt;
 char*uncompressed_record=0,*compressed_record=0,*parsed_record;
 unsigned features,record_size,compressed_record_size;int ret=0;
 if(!rec_num)return!0;
 if(get_rec_list(db))return!0;printf("Header end: %lu\n",pos0=78+8*rec_num);
 printf("Rec. No 0: %lu\n",pos=get_rec_pos(0));
 if(pos>pos0+2)
  printf("Note: rec. No 0 too far (%li bytes) from header end.",pos-pos0);
 if(pos<pos0)
 {printf("Note: rec. No 0 begins in the header.\n"
         " The file is certainly not a PalmOS database.");return!0;
 }
 for(i=1;i<rec_num;i++)
 {if(verbous)printf("Rec. No %u: %lu\n",i,pos=get_rec_pos(i));
  if(pos<pos0)
   printf("Note: The record begins before the previous ends.\n"
	  " I do not support databases with overlapping records.\n"
	  " Is the file really a PalmOS database?\n");pos0=pos;
 }pos=78+8*rec_num;pos0=get_rec_pos(0);
 while(pos<pos0)
 {getc_counted(db);pos++;printf("skip a byte\n");}
 if(check_fopen(&txt,f,"wb"))return!0;
 {unsigned ary,vol_num,vol,rec_num;
  unsigned long pos1=get_rec_pos(1);
  printf("record 0 length %lu (pos %lu)\n",pos1-byte_minor,byte_minor);
  features=get_hu(db);record_size=get_hu(db);
  compressed_record_size=record_size*1.01+12;
  ary=get_hu(db);vol_num=get_hu(db);vol=get_hu(db);
  content_records=get_hu(db);
  printf("features: 0x%4.4X",features);
  if(features)
  {int vex=0;unsigned f_=features;printf(" (");
   if(compression_bit&f_)
   {vex=!0;printf("compressed");f_&=~compression_bit;}
   if(upcoding_table_bit&f_)
   {if(vex)printf(" ");printf("upcoding");f_&=~upcoding_table_bit;}
   if(f_){if(vex)printf(" ");printf("[unknown]");}
   printf(")");
  }printf("\n");
  printf("uncompressed record size: %u maxumum\n",record_size);
  printf("arity:  %u\n",ary);
  printf("Volumes in dictionary:  %u\n",vol_num);
  printf("Current volume:         %u\n",vol);
  printf("First content record:   %u\n",content_records);
  for(i=0;i<ary;i++)
  {rec_num=get_hu(db);if(!i)content_records=rec_num-content_records;
   printf("First %u-ary index:    %u\n",i+1,rec_num);
  }
  {int b='\n';
   printf("Comment: `");
   while(byte_minor<pos1)
   {if(b=='\n')putc('#',txt);c=getc_counted(db);putchar(c);
    putc(c,txt);b=c;
   }putc('\n',txt);printf("'\n");
  }
 }printf("content records: %u\n",content_records);
 if(features&compression_bit)
 {uncompressed_record=malloc(record_size);
  if(!uncompressed_record)
  {fprintf(stderr,"can't allocate %u bytes for record buffer\n",
    record_size);ret=!0;goto exit;
  }
 }
 compressed_record=malloc(compressed_record_size);
 if(!compressed_record)
 {fprintf(stderr,"can't allocate %u bytes for record buffer\n",
   compressed_record_size);ret=!0;goto exit;
 }
 for(i=0;i<content_records;i++)
 {unsigned long ri;unsigned decompressed_size=0;
  pos0=get_rec_pos(i+2)-get_rec_pos(i+1);
  if(features&compression_bit)
  {decompressed_size=get_hu(db);pos0-=palm_ushort_size;}
  for(ri=0;ri<pos0;ri++)compressed_record[ri]=getc_counted(db);
  if(features&compression_bit)
  {uLongf dest_len=/*record_size*/decompressed_size,source_len=compressed_record_size;
   int res=uncompress((Bytef*)uncompressed_record,&dest_len,
     (const Bytef*)compressed_record,source_len);
   parsed_record=uncompressed_record;
   if(res!=Z_OK)
   {fprintf(stderr,"can't uncompress content record No. %u\n",i);
    ret=!0;goto exit;
   }
  }else parsed_record=compressed_record;
  {unsigned j,articles,p=0;
   articles=sget_hu(parsed_record+p);p+=palm_ushort_size;
   if(verbous)printf("content record (%u): articles %u\n",i+1,articles);
   for(j=0;j<articles;j++)
   {unsigned ptr=sget_hu(parsed_record+p);p+=palm_ushort_size;
    if(verbous)printf("%u ptr: %u\n",j,ptr);
   }
   for(j=0;j<articles;j++)
   {if(verbous)printf("article %u at %u: title `",j,p);
    while((c=parsed_record[p++]))
    {putc(c,txt);if(verbous)putchar(c);}
    putc('\n',txt); 
    if(verbous)printf("'; gramm.marks `");
    while((c=parsed_record[p++]))
    {putc(c,txt);if(verbous)putchar(c);}
    putc('\n',txt);if(verbous)printf("'; body `");
    while((c=parsed_record[p++]))
    {putc(c,txt);if(verbous)putchar(c);}
    putc('\n',txt);if(verbous)printf("'\n");
   }
  }
 }i++;
 if(verbous)while(1)
 {unsigned ary,items,vol,rec,ptr,j;static unsigned rn;unsigned long pos0=byte_minor;
  ary=get_hu(db);items=get_hu(db);if(feof(db))break;
  printf("record %u: idx %u: %u-ary, %u items (at %lu)\n",i++,rn++,ary,items,byte_minor);
  for(j=0;j<items;j++)
  {vol=get_hu(db);rec=get_hu(db);ptr=get_hu(db);
   printf("item %u: volume %u, record %u, title at %u\n",j,vol,rec,ptr);
  }
  for(j=0;j<items;j++)
  {int c;
   printf("item %u at %lu title: `",j,byte_minor-pos0);
   while((c=getc_counted(db)))putchar(c);
   printf("'\n");
  }
 }
exit:
 if(uncompressed_record)
 {free(uncompressed_record);uncompressed_record=0;}
 if(compressed_record)
 {free(compressed_record);compressed_record=0;}
 fclose(txt);return ret;
}
void
free_mem(void){if(rec_list)free(rec_list);}
int
main(int argc,char**argv)
{FILE*db;usage();if(argc<3){fprintf(stderr,"wrong usage\n");return 1;}
 if(argc>3)verbous=!0;
 if(check_fopen(&db,argv[1],"rb"))return 2;byte_minor=byte_maior=0;
 printf("Converting %s to %s...\n",argv[1],argv[2]);
 read_header(db);write_list(db,argv[2]);free_mem();
 fclose(db);return 0;
}
