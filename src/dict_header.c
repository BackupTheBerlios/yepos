#include<PalmOS.h>/*yepos the PalmOS dictionary: dictionary header parsing*/
#include"dict_header.h"
#include"../include/signs.h"
#include"control_ids.h"
#define NOZLIBDEFS
#include<SysZLib.h>
static unsigned char default_sort_table[sort_table_length];
void
init_default_sort_table(void)
{static const char s[]=
  "aA bB cC dD eE fF gG hH iI jJ kK lL mM\n"
  "nN oO pP qQ rR sS tT uU vV wW xX yY zZ";
 unsigned char*st=default_sort_table;
 int i,assigned=1;
 for(i=0;s[i];i++)
 {while((!i||s[i-1]=='\n')&&s[i]==' ')
   while(s[++i]&&s[i-1]!='\n');
  if(s[i]==' '||s[i]=='\n'){assigned++;if(s[i]==' ')while(s[++i]==' ');}
  if(s[i]&&s[i]!='\n')
   st[(unsigned char)(s[i])&sort_table_mask]=assigned;
 }
 for(i=1;i<sort_table_length;i++)if(st[i])
  st[i]+=sort_table_length-assigned-1;
 assigned=0;
 for(i=1;i<sort_table_length;i++)if(!st[i])
  st[i]=++assigned;
}
static int
load_dh(struct dict_header*d,unsigned card,LocalID id)
{const char*p;unsigned rec0_size,so;const unsigned*up;if(!d)return!0;
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
 so=0;
 if(d->features&sort_table_bit)
 {const unsigned*sizes=d->ary_records+d->ary+1;
  so=*sizes+sizes[1]+2;
  d->sort_table=(const unsigned char*)(sizes+*sizes+2);
  if(sort_table_length!=sizes[1])so=0;
 }
 if(!so)d->sort_table=default_sort_table;
 d->comment=((const char*)(d->ary_records+d->ary+1))+so;
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
