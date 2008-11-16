#yepos: a PalmOS dictionary
#Makefile for PalmOS program
CC = m68k-palmos-gcc
LD = m68k-palmos-gcc
CPPFLAGS = -I../extern -I. -DNEWGCC
CFLAGS = -Wall -O2
Lang=lat ru en
TARGET=yepos
bin=bin
o=o
Macul=${o}/macul
PRC=$(foreach lang, $(Lang), bin/$(TARGET)_$(lang).prc)
all: ${Macul}/dirs.stamp ${PRC}
.PHONY: all clean
.SECONDARY:$(foreach lang,$(Lang),$(Macul)/$(lang)/$(TARGET).rcp\
 $(Macul)/$(lang)/bin.stamp)
${Macul}/%/${TARGET}.rcp:langs/%.rcp ${TARGET}.rcp.in
	cat $^>$@
clean:
	rm -fr $(DIRS)
DIRS =${bin} ${o} $(Macul) $(foreach lang, $(Lang), $(Macul)/$(lang))
$(Macul)/dirs.stamp:
	mkdir $(DIRS)
	touch $@
${bin}/${TARGET}_%.prc: o/${TARGET} ${TARGET}.def ${Macul}/%/bin.stamp
	build-prc -o $@ ${TARGET}.def $< ${Macul}/$*/*.bin
${Macul}/%/bin.stamp:${Macul}/%/${TARGET}.rcp control_ids.h ${Macul}/dirs.stamp
	pilrc -q -L $* $< ${Macul}/$*
	touch $@
${o}/${TARGET}: ${o}/yepos.o ${o}/show_battery.o ${o}/prefs.o ${o}/settings.o
	${LD} ${LDFLAGS} -o $@ $^
${o}/${TARGET}.o: yepos.c control_ids.h show_battery.h globals.h\
 prefs.h ../include/signs.h enums.h settings.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/show_battery.o: show_battery.c show_battery.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/prefs.o: prefs.c prefs.h globals.h ../include/signs.h enums.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/settings.o: settings.c settings.h control_ids.h globals.h enums.h\
 ../include/signs.h prefs.h show_battery.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
#Copyright (C) 2008 Ineiev<ineiev@users.sourceforge.net>, super V 93
#
#yepos is free software; you can redistribute it and/or modify
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
