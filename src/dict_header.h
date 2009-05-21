/*yepos the PalmOS dictionary: dictionary header structure
Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93

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
struct dict_header
{DmOpenRef db;MemHandle rec0;
 unsigned features,record_size,ary,volumes,vol,comment_size;
 const unsigned*ary_records;const char*comment;
 const unsigned char*sort_table;
};
int load_dict_header(struct dict_header*,unsigned card,LocalID id);
void close_dict_header(struct dict_header*);
void init_default_sort_table(void);
enum dict_header_lengths
{dh_bits_per_byte=8,
 sort_table_length=1<<dh_bits_per_byte,
 sort_table_mask=(1<<dh_bits_per_byte)-1
};
