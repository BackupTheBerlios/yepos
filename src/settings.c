#include<PalmOS.h>/*yepos the PalmOS dictionary: settings form functions*/
#include"../include/signs.h"
#include"settings.h"
#include"control_ids.h"
#include"dict_header.h"
#include"enums.h"
#include"globals.h"
#include"prefs.h"
#include"show_battery.h"
enum local_constants{info_line_inc=4};
static int upper_db,selected,change_flag,info_line_no,comment_top_reached;
static struct dict_header dh;
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
}static int
load_selected_header(void)
{int n;struct database_handle**h=get_database_list(&n);
 if(selected>n)return!0;if(dh.db)close_dict_header(&dh);
 return load_dict_header(&dh,h[selected]->card,h[selected]->id);
}static void
draw_tail(int visible,int y)
{if(visible)WinDrawChars("...",3,0,y);
 else WinDrawChars("      ",6,0,y);
}static int
draw_resource_string(int id,int x,int y)
{MemHandle mh=DmGetResource('tSTR',id);const char*s;
 if(!mh)return!0;s=MemHandleLock(mh);if(!s)return!0;
 WinDrawChars(s,StrLen(s),0,y);MemHandleUnlock(mh);return 0;
}static int
comment_area_top(void)
{int n;get_database_list(&n);if(n>5)n=5;return 12+n*14;}
static void
clr_db_comment(void)
{static struct RectangleType r={{0,0},{screen_width,0}};
 int y;y=comment_area_top()+1;
 r.topLeft.y=y;r.extent.y=137-y;WinEraseRectangle(&r,0);
}static void
show_selected_database(FormType*f)
{int i,y,comment_size,incy=11,max_y=130,line=0;
 const char*comment;y=comment_area_top();
 if(info_line_no<=line)
 {if(!draw_resource_string(Dictionary_No_Str_id,0,y))
  {char s[17];StrIToA(s,selected);WinDrawChars(s,StrLen(s),100,y);
   y+=incy;
  }
 }line++;
 if(info_line_no<=line)
 {if(!draw_resource_string(Db_Features_Str_id,0,y))
  {char s[17];StrIToH(s,dh.features);s[3]='x';
   WinDrawChars(s+2,6,100,y);y+=incy;
  }
 }line++;
 comment=dh.comment;comment_size=dh.comment_size;
 for(i=0;0<comment_size;i++)
 {if(comment[i]=='\n'||i==comment_size)
  {if(line++>=info_line_no)
   {WinDrawChars(comment,i,0,y);y+=incy;}
   comment+=i+1;comment_size-=i+1;i=-1;
  }if(y>max_y)break;
 }draw_tail(comment_size>0,135);comment_top_reached=comment_size<=0;
}static void
return_from_settings(void)
{if(!change_flag)skip_next_redraw();
 if(dh.db)close_dict_header(&dh);at_close_app(0);goto_form(MainForm_id);
}static void
on_enter(void)
{FormType*f;unsigned long tic=TimGetTicks();int n;
 f=get_current_form();selected=get_current_db_idx();get_database_list(&n);
 if(n>Dictionary_Pushbuttons_number&&selected>Dictionary_Pushbuttons_number-1)
 {upper_db=selected-Dictionary_Pushbuttons_number/2;
  if(upper_db+Dictionary_Pushbuttons_number>n)
   upper_db=n-Dictionary_Pushbuttons_number;
 }
 show_buttons(f);FrmDrawForm(f);show_battery(!0);
 at_close_app(return_from_settings);
 if(!load_selected_header())show_selected_database(f);
 {char s[17];StrIToA(s,TimGetTicks()-tic);
  WinDrawChars(s,StrLen(s),screen_width-StrLen(s)*5-2,-2);
 }change_flag=info_line_no=0;
}static int
process_push(int n)
{FormType*f=get_current_form();UInt16 idx;ControlType*b;
 int db_num,i;get_database_list(&db_num);
 n-=Dictionary_Pushbutton_First_id;
 if(upper_db+n==selected&&(n||!upper_db)
    &&(upper_db+Dictionary_Pushbuttons_number>db_num))
  return 0;
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
 if(!load_selected_header())
 {info_line_no=0;clr_db_comment();show_selected_database(f);}
 return 0;
}static int
process_pendown(int x,int y)
{int y0=comment_area_top(),y1=135;if((y>y1&&x>20)||y<y0)return 0;
 if(y>(y1+y0)/2)
 {if(!comment_top_reached)
  {info_line_no+=info_line_inc;clr_db_comment();
   show_selected_database(get_current_form());
  }
 }else if(info_line_no)
 {info_line_no-=info_line_inc;if(info_line_no<0)info_line_no=0;
  clr_db_comment();show_selected_database(get_current_form());
 }return!0;
}Boolean
settings_form_handler(EventType*e)
{switch(e->eType)
 {case frmOpenEvent:on_enter();break;
  case penDownEvent:return process_pendown(e->screenX,e->screenY);
  case ctlSelectEvent:
   switch(e->data.ctlSelect.controlID)
   {case OK_Button_id:
     if(get_current_db_idx()!=selected)
     {load_database(selected);change_flag=!0;}
     return_from_settings();break;
    case Cancel_Button_id:
     return_from_settings();break;
    case About_Button_id:FrmHelp(About_Str_id);break;
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
