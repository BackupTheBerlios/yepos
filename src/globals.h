/*yepos the PalmOS dictionary: global data controls
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
void goto_form(int id);
FormType*get_current_form(void);
struct database_handle
{char name[db_name_size+1];UInt16 card;LocalID id;};
struct database_handle**get_database_list(int*items_number);
int get_current_db_idx(void);int load_database(int index);
