#include<PalmOS.h>/*yepos the PalmOS dictionary: settings form functions*/
#include"settings.h"
#include"control_ids.h"
#include"enums.h"
#include"globals.h"
#include"../include/signs.h"
#include"prefs.h"
#include"show_battery.h"
static int upper_db,selected;
static void
show_buttons(FormType*f)
{int n,m,i,i0=0;struct database_handle**h;
 h=get_database_list(&n);
 m=Dictionary_Pushbuttons_number;
 if(upper_db)
 {UInt16 idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id);
  ControlType*b=FrmGetObjectPtr(f,idx);CtlSetLabel(b,"...");
  CtlSetValue(b,0);i0++;n-=upper_db;
 }if(m>n)m=n;
 for(i=i0;i<m;i++)
 {UInt16 idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id+i);
  ControlType*b=FrmGetObjectPtr(f,idx);CtlSetLabel(b,h[i+upper_db]->name);
  CtlSetValue(b,selected-upper_db==i);
 }
 if(m==n)for(;i<Dictionary_Pushbuttons_number;i++)
 {UInt16 idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id+i);
  ControlType*b=FrmGetObjectPtr(f,idx);CtlHideControl(b);
 }else
 {UInt16 idx=FrmGetObjectIndex(f,
   Dictionary_Pushbutton_First_id+Dictionary_Pushbuttons_number-1);
  ControlType*b=FrmGetObjectPtr(f,idx);CtlSetLabel(b,"...");
  CtlSetValue(b,0);
 }
}static void
show_selected_database(FormType*f)
{
}static void
on_enter(void)
{FormType*f;unsigned long tic=TimGetTicks();
 f=get_current_form();selected=get_current_db_idx();
 show_buttons(f);FrmDrawForm(f);show_battery(!0);
 show_selected_database(f);
 {char s[17];StrIToA(s,TimGetTicks()-tic);
  WinDrawChars(s,StrLen(s),160-StrLen(s)*5-2,-2);
 }
}static int
process_push(int n)
{FormType*f=get_current_form();UInt16 idx;ControlType*b;
 int db_num,i;get_database_list(&db_num);
 n-=Dictionary_Pushbutton_First_id;
 if(upper_db&&!n)
 {idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id);
  b=FrmGetObjectPtr(f,idx);CtlSetValue(b,0);
  upper_db-=Dictionary_Pushbuttons_number-3;if(upper_db<0)upper_db=0;
  idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_Last_id);
  b=FrmGetObjectPtr(f,idx);CtlSetValue(b,0);
  show_buttons(f);return!0;
 }
 if(db_num-upper_db>Dictionary_Pushbuttons_number
  &&n==Dictionary_Pushbuttons_number-1)
 {idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_Last_id);
  b=FrmGetObjectPtr(f,idx);CtlSetValue(b,0);
  upper_db+=Dictionary_Pushbuttons_number-3;
  if(upper_db>db_num-Dictionary_Pushbuttons_number)
   upper_db=db_num-Dictionary_Pushbuttons_number;
  idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id);
  b=FrmGetObjectPtr(f,idx);CtlSetValue(b,0);
  show_buttons(f);return!0;
 }
 for(i=0;i<Dictionary_Pushbuttons_number;i++)
 {idx=FrmGetObjectIndex(f,Dictionary_Pushbutton_First_id+i);
  b=FrmGetObjectPtr(f,idx);CtlSetValue(b,i==n);
 }selected=upper_db+n;
 return 0;
}Boolean
settings_form_handler(EventType*e)
{switch(e->eType)
 {case frmOpenEvent:on_enter();break;
  case ctlSelectEvent:
   switch(e->data.ctlSelect.controlID)
   {case OK_Button_id:if(get_current_db_idx()!=selected)load_database(selected);
     goto_form(MainForm_id);break;
    case Cancel_Button_id:
     goto_form(MainForm_id);break;
    default:
     if(e->data.ctlSelect.controlID>=Dictionary_Pushbutton_First_id
        &&e->data.ctlSelect.controlID<=Dictionary_Pushbutton_Last_id)
      process_push(e->data.ctlSelect.controlID);
     else return 0;
   }break;
  default:return 0;
 }return!0;
}/*Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93

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
