/*yepos the PalmOS dictionary: zlib error alert header*/
#include<StringMgr.h>
#include<Form.h>
#define NOZLIBDEFS
#include<SysZLib.h>
#include"zlib_error_alert.h"
#include"control_ids.h"
unsigned
zlib_error_alert(int error,const char*function)
{const char*err_code;char s[0x11];
 switch(error)
 {case Z_OK:err_code="Z_OK";break;
  case Z_STREAM_END:err_code="Z_STREAM_END";break;
  case Z_NEED_DICT:err_code="Z_NEED_DICT";break;
  case Z_ERRNO:err_code="Z_ERRNO";break;
  case Z_STREAM_ERROR:err_code="Z_STREAM_ERROR";break;
  case Z_DATA_ERROR:err_code="Z_DATA_ERROR";break;
  case Z_MEM_ERROR:err_code="Z_MEM_ERROR";break;
  case Z_BUF_ERROR:err_code="Z_BUF_ERROR";break;
  case Z_VERSION_ERROR:err_code="Z_VERSION_ERROR";break;
  default:err_code=s;StrIToA(s,error);
 }return FrmCustomAlert(ZLib_Error_Alert_id,function,err_code," ");
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
