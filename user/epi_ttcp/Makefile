ROMFS_DIR = $(ROOTDIR)/romfs

all: epi_ttcp libgcc_s.so

clean:

romfs:
	$(ROMFSINST) /lib/libgcc_s.so
	cd $(INSTALLDIR)/lib && ln -sf libgcc_s.so libgcc_s.so.1
	$(ROMFSINST) /usr/bin/epi_ttcp
