# convert stardict2txt output into yeposc input format via sed(1)
# Usage:
# stardict2txt serb2croat.ifo
# sed -f star2y.sed <serb2croat.txt >s2c_y.txt
#  where
#   stardict2txt is a program coming with stardict-tools distribution;
#   serb2croat.ifo is stardict database description file name;
#    (there also should be serb2croat.dict and serb2croat.idx files:
#     typical uncompression command is
#     `gzip -dc <serb2croat.dict.dz >serb2croat.dict')
/\([^\t]*\)\t\1.*\\n/s/^\([^\t]*\)\t\1\(.*\)\\n/\1\n\2\n/
s/\t/\n\n/;s/\\n[ \t]*/ |/g
#Copyright (C) 2009 Ineiev<ineiev@users.berlios.de>, super V 93
# (this amount of code is hardly copyrightable,
#  but this program is part of yepos)
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program. If not, see <http://www.gnu.org/licenses/>.
