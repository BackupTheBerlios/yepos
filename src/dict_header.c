#include<PalmOS.h>/*yepos the PalmOS dictionary: dictionary header parsing*/
#include"dict_header.h"
#include"../include/signs.h"
#include"control_ids.h"
#define NOZLIBDEFS
#include<SysZLib.h>
/* this is old stuff; to be deleted
static int
parse_header(int verbous)
{char s[0x33];char*p;unsigned*up;int y=0,incy=11,i,j;
 record0=DmQueryRecord(current_db,0);
 if(!record0)return!0;p=MemHandleLock(record0);if(!p)return!0;
 rec0_size=MemPtrSize(p);
 up=(unsigned*)p;features=up;
 if(*features&~implemented_features_bits)
 {FrmAlert(Unimplemented_Alert_id);return!0;}
 if(*features&compression_bit&&!ZLibRef)
 {FrmAlert(No_Zlib_Alert_id);return!0;}
 record_size=up+1;ary=up+2;
 volumes=up+3;vol=up+4;ary_records=up+5;
 db_comment=(const char*)(ary_records+*ary+1);
 comment_size=rec0_size-(db_comment-p);
 if(!verbous)return 0;
 StrCopy(s,"Database ");j=StrLen(s);
 for(i=0;i<comment_size;i++,j++)
 {s[j]=db_comment[i];
  if(s[j]=='\n')
  {s[j]=0;WinDrawChars(s,StrLen(s),0,y);y+=incy;j=-1;}
 }
 if(db_comment[i-1]!='\n')
 {s[j]=0;WinDrawChars(s,StrLen(s),0,y);y+=incy;}
 StrCopy(s,"features: ");StrIToH(s+StrLen(s),*features);
 StrCat(s,"; arity: ");StrIToA(s+StrLen(s),*ary);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 StrCopy(s,"content record size: ");
 StrIToA(s+StrLen(s),*record_size);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 StrCopy(s,"volumes: ");StrIToA(s+StrLen(s),*volumes);
 StrCat(s,"; current volume: ");StrIToA(s+StrLen(s),*vol);
 WinDrawChars(s,StrLen(s),0,y);y+=incy;
 for(i=0;i<=*ary;i++)
 {StrIToA(s,i);StrCat(s,"-ary records begin with ");
  StrIToA(s+StrLen(s),ary_records[i]);
  WinDrawChars(s,StrLen(s),0,y);y+=incy;
 }return 0;
}*/
static int
load_dh(struct dict_header*d,unsigned card,LocalID id)
{const char*p;unsigned rec0_size;const unsigned*up;if(!d)return!0;
 d->db=DmOpenDatabase(card,id,dmModeReadOnly);
 if(!d->db){FrmAlert(Fail_Opening_Alert_id);return!0;}
 d->rec0=DmQueryRecord(d->db,0);if(!d->rec0)return!0;
 p=MemHandleLock(d->rec0);if(!p)return!0;rec0_size=MemPtrSize((char*)p);
 up=(const unsigned*)p;d->features=*up;
 if(d->features&~implemented_features_bits)
 {FrmAlert(Unimplemented_Alert_id);return!0;}
 if((d->features&compression_bit)&&!ZLibRef)
 {FrmAlert(No_Zlib_Alert_id);return!0;}
 d->record_size=up[1];d->ary=up[2];d->volumes=up[3];
 d->vol=up[4];d->ary_records=up+5;
 d->comment=(const char*)(d->ary_records+d->ary+1);
 d->comment_size=rec0_size-(d->comment-p);return 0;
}int 
load_dict_header(struct dict_header*d,unsigned card,LocalID id)
{if(!load_dh(d,card,id))return 0;close_dict_header(d);return!0;}
void
close_dict_header(struct dict_header*d)
{if(!d)return;if(d->rec0){MemHandleUnlock(d->rec0);d->rec0=0;}
 if(d->db){DmCloseDatabase(d->db);d->db=0;}
}/*Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93

yepos is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.*/
