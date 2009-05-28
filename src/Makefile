#yepos: a PalmOS dictionary
#Makefile for PalmOS programs
CC = m68k-palmos-gcc
LD = m68k-palmos-gcc
CPPFLAGS = -I../extern -I. -DNEWGCC
CFLAGS = -Wall -O2
yepos_version=post-0.2-rc0
#whether to build executables for all languages (not only Latin)
BUILD_LANGS=1
#whether to build test prcs
BUILD_TESTS=0
Lang=lat
ifneq (${BUILD_LANGS},0)
Lang+=ru en
endif
TARGET=yepos
bin=bin
o=o
Macul=${o}/macul
PRC=$(foreach lang, $(Lang), ${bin}/$(TARGET)_$(lang).prc)
test_prc=${bin}/mem_ory.test.prc
ifneq (${BUILD_TESTS},0)
 PRC+=${test_prc}
endif
all: ${Macul}/dirs.stamp ${PRC}
.PHONY: all clean
.SECONDARY:$(foreach lang,$(Lang),$(Macul)/$(lang)/$(TARGET).rcp\
 $(Macul)/$(lang)/bin.stamp)
${Macul}/%/${TARGET}.rcp:langs/%.rcp ${TARGET}.rcp.in
	cat $^|sed "s/\<YEPOS_VERSION_STRING\>[^\"]*\"/\"${yepos_version}/">$@
clean:
	rm -fr $(DIRS)
DIRS =${bin} ${o} $(Macul) $(foreach lang, $(Lang), $(Macul)/$(lang))
$(Macul)/dirs.stamp:
	mkdir $(DIRS)
	touch $@
${bin}/${TARGET}_%.prc: ${o}/${TARGET} ${TARGET}.def ${Macul}/%/bin.stamp
	build-prc -o $@ ${TARGET}.def $< ${Macul}/$*/*.bin
${bin}/mem_ory.test.prc: ${o}/mem_ory.test mem_ory.test.def
	build-prc -o $@ mem_ory.test.def $<
${Macul}/%/bin.stamp:${Macul}/%/${TARGET}.rcp control_ids.h ${Macul}/dirs.stamp
	pilrc -q -L $* $< ${Macul}/$*
	touch $@
${o}/${TARGET}: ${o}/yepos.o ${o}/show_battery.o ${o}/prefs.o ${o}/settings.o\
 ${o}/zlib_error_alert.o ${o}/mem_ory.o ${o}/dict_header.o
	${LD} ${LDFLAGS} -o $@ $^
${o}/mem_ory.test: ${o}/mem_ory.test.o ${o}/mem_ory.o
	${LD} ${LDFLAGS} -o $@ $^
${o}/${TARGET}.o: yepos.c control_ids.h show_battery.h globals.h\
 prefs.h ../include/signs.h enums.h settings.h zlib_error_alert.h\
 mem_ory.h dict_header.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/show_battery.o: show_battery.c show_battery.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/prefs.o: prefs.c prefs.h globals.h ../include/signs.h enums.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/settings.o: settings.c settings.h ../include/signs.h\
 control_ids.h dict_header.h globals.h enums.h prefs.h show_battery.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/dict_header.o: dict_header.c dict_header.h control_ids.h\
 ../include/signs.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/mem_ory.o: mem_ory.c mem_ory.h ../include/signs.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/mem_ory.test.o: mem_ory.test.c mem_ory.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
${o}/zlib_error_alert.o: zlib_error_alert.c zlib_error_alert.h control_ids.h
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<
#Copyright (C) 2008, 2009 Ineiev<ineiev@users.berlios.de>, super V 93
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
