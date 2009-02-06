#include<PalmOS.h>/*yepos the PalmOS dictionary: preferences functions*/
#include"prefs.h"
#include"../include/signs.h"
#include"enums.h"
#include"globals.h"
#include"control_ids.h"
enum local_constants{prefs_id=0,prefs_version=0};
static char*lookup,db_name[db_name_size+1];
void
set_lookup(const char*l)
{if(lookup)MemPtrFree(lookup);lookup=0;
 if(!(l&&StrLen(l)))return;
 lookup=MemPtrNew(StrLen(l)+1);if(lookup)StrCopy(lookup,l);
}const char*
get_lookup(void){return lookup;}
static int
try_db_name(void)
{int i,n;struct database_handle**db=get_database_list(&n);
 if(*db_name)for(i=0;i<n;i++)if(!StrCompare(db[i]->name,db_name))
  if(!load_database(i))return 0;
 *db_name=0;if(n>0)return load_database(0);
 FrmAlert(No_Dictionary_Alert_id);return!0;
}enum aux_flags_bits{list_mode_bit=0};
int
load_preferences(void)
{UInt16 prefs_size=0;int version,n;char*p,*_;
 version=PrefGetAppPreferences(CREATOR,prefs_id,0,&prefs_size,0);
 if(version!=prefs_version)return try_db_name();
 if(prefs_size<db_name_size+1)return try_db_name();
 p=MemPtrNew(prefs_size);if(!p)return try_db_name();_=p;
 version=PrefGetAppPreferences(CREATOR,prefs_id,p,&prefs_size,0);
 if(version!=prefs_version){MemPtrFree(p);return try_db_name();}
 n=db_name_size;MemMove(db_name,_,n);_+=n;prefs_size-=n;db_name[n]=0;
 if(try_db_name())return!0;
 if(prefs_size>1)
 {n=StrLen(_)+1;lookup=MemPtrNew(n);
  if(lookup)MemMove(lookup,_,n);prefs_size-=n;_+=n;
 }if(prefs_size>0)
 {char aux_flags=*_;set_list_mode(2*!!(aux_flags&(1<<list_mode_bit)));
  prefs_size-=1;_+=1;
 }MemPtrFree(p);return 0;
}static int
prefs_size(void)
{int size=db_name_size;const char*l=get_lookup();
 if(l)size+=StrLen(l);size+=1;size+=1;return size;
}void
save_preferences(void)
{int size=prefs_size(),n;char*p=MemPtrNew(size),*_=p;const char*l;
 const char*dbn;struct database_handle**dbl;char aux_flags;
 if(!p){if(lookup)MemPtrFree(lookup);lookup=0;return;}
 l=get_lookup();dbl=get_database_list(&n);dbn=db_name;
 if(dbl)dbn=dbl[get_current_db_idx()]->name;
 MemMove(_,dbn,db_name_size);_+=db_name_size;
 if(l){StrCopy(_,l);_+=StrLen(l);if(lookup)MemPtrFree(lookup);lookup=0;}
 *_++=0;aux_flags=0;if(get_list_mode())aux_flags|=1<<list_mode_bit;
 *_=aux_flags;
 PrefSetAppPreferences(CREATOR,prefs_id,prefs_version,p,size,0);
 MemPtrFree(p);
}/*Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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
