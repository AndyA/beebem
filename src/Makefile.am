# Makefile.am for BeebEm

PACKAGE = @PACKAGE@
VERSION = @VERSION@

SUBDIRS = gui

#INCLUDE = -I@top_srcdir@/src/gui

#INCLUDES        = @GTK_CFLAGS@ -I@top_srcdir@/src/gui
#LDADD           = @GTK_LIBS@ @top_srcdir@/src/gui/libeg.a

INCLUDES        = @GTK_CFLAGS@ -I@top_srcdir@/src/gui -I@top_srcdir@/src/
LDADD           = @GTK_LIBS@ @top_srcdir@/src/gui/libeg.a


CXX = @CXX@ -Wall -DDATA_DIR=\"$(pkgdatadir)\" 
CC = @CC@ -Wall -DDATA_DIR=\"$(pkgdatadir)\"

bin_PROGRAMS   = beebem
beebem_SOURCES =main.cpp log.c sdl.cpp windows.cpp beebem_pages.cpp \
		beebem_pages_disks.cpp beebem_pages_disks.h \
		beebem_pages.h fake_registry.c fake_registry.h \
		user_config.c user_config.h \
		serialdevices.cpp serialdevices.h \
		csw.cpp csw.h \
		6502core.cpp atodconv.cpp avi.cpp beebmem.cpp beebsound.cpp \
		beebwin.cpp cregistry.cpp debug.cpp disc1770.cpp disc8271.cpp \
		econet.cpp sasi.cpp scsi.cpp serial.cpp speech.cpp sysvia.cpp \
		tube.cpp uef.cpp uefstate.cpp userkybd.cpp uservia.cpp via.cpp \
		video.cpp z80.cpp z80_support.cpp z80dis.cpp i386dasm.cpp i86.cpp \
		teletext.cpp \
		\
		main.h types.h log.h line.h sdl.h \
		hardware.h hardware.cpp \
		windows.h \
		6502core.h atodconv.h avi.h beebem.h \
		beebemrc.h beebmem.h beebsound.h beebwin.h \
		cregistry.h debug.h disc1770.h \
		disc8271.h econet.h \
		port.h sasi.h scsi.h serial.h \
		speech.h sysvia.h tube.h uef.h \
		uefstate.h userkybd.h uservia.h \
		via.h viastate.h video.h \
		z80.h z80mem.h \
		zlib/zlib.h zlib/zconf.h \
		ea.h i86.h instr86.h osd_cpu.h teletext.h

