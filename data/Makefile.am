# Makefile for installing and uninstalling all BeebEm data files

SHELL=@SHELL@

pkgdata_DATA = config/econet.cfg config/phroms.cfg config/roms.cfg config/roms_torch.cfg \
		config/roms_bbc.cfg \
		\
		media/discs/econet_level_1_utils.ssd media/discs/econet_level_2_utils.ssd \
		media/discs/games.ssd media/discs/master_welcome.adl media/discs/test.ssd \
		media/discs/welcome.ssd media/discs/bbcmaster512-disc1-dosplusboot.adl \
		media/discs/cpm_utilities_disc.dsd \
		\
		media/scsi/sasi0.dat \
		media/scsi/scsi0.dat media/scsi/scsi0.dsc \
		media/scsi/scsi1.dat media/scsi/scsi1.dsc \
		media/scsi/scsi2.dat media/scsi/scsi2.dsc \
		media/scsi/scsi3.dat media/scsi/scsi3.dsc \
		\
		media/tapes/test.uef \
		\
		phroms/phroma phroms/phromsus.txt phroms/phromus \
		\
		resources/icon.bmp resources/relayoff.snd resources/relayon.snd \
		resources/teletext.fnt resources/font10x16.bmp resources/font5x8.bmp \
		\
		roms/bbc/adfs-1.30.rom roms/bbc/basic2.rom roms/bbc/dnfs.rom \
		roms/bbc/mcp120.cbl roms/bbc/os12.rom roms/bbc/wdfs.rom \
		roms/bbc/ats-3.0-1.rom \
		\
		roms/bbcint/basic2.rom roms/bbcint/dnfs.rom roms/bbcint/ibos.rom \
		roms/bbcint/os12.rom roms/bbcint/wdfs.rom \
		\
		roms/bplus/adfs-1.30.rom roms/bplus/b+mos.rom roms/bplus/basic2.rom \
		roms/bplus/dfs-2.26.rom roms/bplus/wdfs.rom \
		\
		roms/m128/adfs.rom roms/m128/adfs1-53.rom roms/m128/anfs-4.25-2201351.rom \
		roms/m128/basic4.rom roms/m128/dfs.rom roms/m128/edit.rom \
		roms/m128/mcp122.abm roms/m128/mos.rom roms/m128/terminal.rom \
		roms/m128/view.rom roms/m128/viewsht.rom roms/m128/ats-3.0-1.rom \
		\
		roms/other/6502tube.rom roms/other/ccpn102.rom roms/other/hibasic3.rom \
		roms/other/z80.rom roms/other/bios.rom \
		\
		state/cmos.ram

EXTRA_DIST = $(pkgdata_DATA)

# $(wildcard *.meta)

install:
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/config
	cp -r $(top_srcdir)/data/config/* $(pkgdatadir)/config
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/media
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/media/discs
	cp -r $(top_srcdir)/data/media/discs/* $(pkgdatadir)/media/discs
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/media/scsi
	cp -r $(top_srcdir)/data/media/scsi/* $(pkgdatadir)/media/scsi
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/media/tapes
	cp -r $(top_srcdir)/data/media/tapes/* $(pkgdatadir)/media/tapes
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/phroms
	cp -r $(top_srcdir)/data/phroms/* $(pkgdatadir)/phroms
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/resources
	cp -r $(top_srcdir)/data/resources/* $(pkgdatadir)/resources
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/state
	cp -r $(top_srcdir)/data/state/* $(pkgdatadir)/state
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms/bbc
	cp -r $(top_srcdir)/data/roms/bbc/* $(pkgdatadir)/roms/bbc
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms/bbcint
	cp -r roms/bbcint/* $(pkgdatadir)/roms/bbcint
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms/bplus
	cp -r roms/bplus/* $(pkgdatadir)/roms/bplus
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms/m128
	cp -r $(top_srcdir)/data/roms/m128/* $(pkgdatadir)/roms/m128
#
	$(SHELL) $(top_srcdir)/mkinstalldirs $(pkgdatadir)/roms/other
	cp -r $(top_srcdir)/data/roms/other/* $(pkgdatadir)/roms/other
#
	chmod -R 755 $(pkgdatadir)

uninstall:
	rm -fv   $(pkgdatadir)/config/econet.cfg
	rm -fv   $(pkgdatadir)/config/phroms.cfg
	rm -fv   $(pkgdatadir)/config/roms.cfg
	rm -fv   $(pkgdatadir)/config/roms_torch.cfg
	rm -fv	 $(pkgdatadir)/config/roms_bbc.cfg
	rmdir -v $(pkgdatadir)/config
#
	rm -fv   $(pkgdatadir)/media/tapes/test.uef
	rmdir -v $(pkgdatadir)/media/tapes
#
	rm -fv	 $(pkgdatadir)/media/scsi/sasi0.dat
#
	rm -fv   $(pkgdatadir)/media/scsi/scsi0.dat
	rm -fv   $(pkgdatadir)/media/scsi/scsi0.dsc
	rm -fv   $(pkgdatadir)/media/scsi/scsi1.dat
	rm -fv   $(pkgdatadir)/media/scsi/scsi1.dsc
	rm -fv   $(pkgdatadir)/media/scsi/scsi2.dat
	rm -fv   $(pkgdatadir)/media/scsi/scsi2.dsc
	rm -fv	 $(pkgdatadir)/media/scsi/scsi3.dat
	rm -fv   $(pkgdatadir)/media/scsi/scsi3.dsc
	rmdir -v $(pkgdatadir)/media/scsi
#
	rm -fv   $(pkgdatadir)/media/discs/bbcmaster512-disc1-dosplusboot.adl
	rm -fv   $(pkgdatadir)/media/discs/cpm_utilities_disc.dsd
	rm -fv   $(pkgdatadir)/media/discs/econet_level_1_utils.ssd
	rm -fv   $(pkgdatadir)/media/discs/econet_level_2_utils.ssd
	rm -fv   $(pkgdatadir)/media/discs/games.ssd
	rm -fv   $(pkgdatadir)/media/discs/master_welcome.adl
	rm -fv   $(pkgdatadir)/media/discs/test.ssd
	rm -fv   $(pkgdatadir)/media/discs/welcome.ssd
	rmdir -v $(pkgdatadir)/media/discs
#
	rmdir -v $(pkgdatadir)/media
#
	rm -fv   $(pkgdatadir)/phroms/phroma
	rm -fv   $(pkgdatadir)/phroms/phromsus.txt
	rm -fv   $(pkgdatadir)/phroms/phromus
	rmdir -v $(pkgdatadir)/phroms
#
	rm -fv   $(pkgdatadir)/resources/font10x16.bmp
	rm -fv   $(pkgdatadir)/resources/font5x8.bmp
	rm -fv   $(pkgdatadir)/resources/icon.bmp
	rm -fv   $(pkgdatadir)/resources/relayoff.snd
	rm -fv   $(pkgdatadir)/resources/relayon.snd
	rm -fv   $(pkgdatadir)/resources/teletext.fnt
	rmdir -v $(pkgdatadir)/resources
#
	rm -fv   $(pkgdatadir)/roms/bbc/adfs-1.30.rom
	rm -fv   $(pkgdatadir)/roms/bbc/ats-3.0-1.rom
	rm -fv   $(pkgdatadir)/roms/bbc/basic2.rom
	rm -fv   $(pkgdatadir)/roms/bbc/dnfs.rom
	rm -fv   $(pkgdatadir)/roms/bbc/mcp120.cbl
	rm -fv   $(pkgdatadir)/roms/bbc/os12.rom
	rm -fv   $(pkgdatadir)/roms/bbc/wdfs.rom
	rmdir -v $(pkgdatadir)/roms/bbc
#
	rm -fv   $(pkgdatadir)/roms/bbcint/basic2.rom
	rm -fv   $(pkgdatadir)/roms/bbcint/dnfs.rom
	rm -fv   $(pkgdatadir)/roms/bbcint/ibos.rom
	rm -fv   $(pkgdatadir)/roms/bbcint/os12.rom
	rm -fv   $(pkgdatadir)/roms/bbcint/wdfs.rom
	rmdir -v $(pkgdatadir)/roms/bbcint
#
	rm -fv   $(pkgdatadir)/roms/bplus/adfs-1.30.rom
	rm -fv   $(pkgdatadir)/roms/bplus/b+mos.rom
	rm -fv   $(pkgdatadir)/roms/bplus/basic2.rom
	rm -fv   $(pkgdatadir)/roms/bplus/dfs-2.26.rom
	rm -fv   $(pkgdatadir)/roms/bplus/wdfs.rom
	rmdir -v $(pkgdatadir)/roms/bplus
#
	rm -fv   $(pkgdatadir)/roms/m128/adfs.rom
	rm -fv   $(pkgdatadir)/roms/m128/adfs1-53.rom
	rm -fv   $(pkgdatadir)/roms/m128/anfs-4.25-2201351.rom
	rm -fv   $(pkgdatadir)/roms/m128/ats-3.0-1.rom
	rm -fv   $(pkgdatadir)/roms/m128/basic4.rom
	rm -fv   $(pkgdatadir)/roms/m128/dfs.rom
	rm -fv   $(pkgdatadir)/roms/m128/edit.rom
	rm -fv   $(pkgdatadir)/roms/m128/mcp122.abm
	rm -fv   $(pkgdatadir)/roms/m128/mos.rom
	rm -fv   $(pkgdatadir)/roms/m128/terminal.rom
	rm -fv   $(pkgdatadir)/roms/m128/view.rom
	rm -fv   $(pkgdatadir)/roms/m128/viewsht.rom
	rmdir -v $(pkgdatadir)/roms/m128
#
	rm -fv   $(pkgdatadir)/roms/other/6502tube.rom
	rm -fv   $(pkgdatadir)/roms/other/bios.rom
	rm -fv   $(pkgdatadir)/roms/other/ccpn102.rom
	rm -fv   $(pkgdatadir)/roms/other/hibasic3.rom
	rm -fv   $(pkgdatadir)/roms/other/z80.rom
	rmdir -v $(pkgdatadir)/roms/other
#
	rmdir -v $(pkgdatadir)/roms
#
	rm -fv   $(pkgdatadir)/state/cmos.ram
	rmdir -v $(pkgdatadir)/state
#
	rmdir -v $(pkgdatadir)
