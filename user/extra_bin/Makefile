ROMFS_DIR = $(ROOTDIR)/romfs

all: chkntfs ufsd.ko

clean:

romfs:
	$(ROMFSINST) /bin/ated
#	$(ROMFSINST) /bin/flash
#	$(ROMFSINST) /bin/reg
	$(ROMFSINST) /usr/sbin/chkntfs
	cp ufsd.ko $(ROMFS_DIR)/usr/lib/ufsd.ko
#	cp disable_pin.scr $(ROMFS_DIR)/etc_ro/ppp/3g/disable_pin.scr
#	$(ROMFSINST) /usr/lib/ufsd.ko
#	$(ROMFSINST) /etc_ro/ppp/3g/disable_pin.scr
#	$(STRIP) libiconv.so.0.0.0
#	$(ROMFSINST) /lib/libiconv.so.0.0.0
#	cd $(INSTALLDIR)/lib && ln -sf libiconv.so.0.0.0 libiconv.so.0
#	cd $(INSTALLDIR)/lib && ln -sf libiconv.so.0.0.0 libiconv.so

