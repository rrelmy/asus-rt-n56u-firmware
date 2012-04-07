ROMFS_DIR = $(ROOTDIR)/romfs

all: chkntfs ufsd.ko

clean:

romfs:
	$(ROMFSINST) /usr/sbin/chkntfs
	cp ufsd.ko $(ROMFS_DIR)/usr/lib/ufsd.ko
#	cp disable_pin.scr $(ROMFS_DIR)/etc_ro/ppp/3g/disable_pin.scr
#	$(ROMFSINST) /usr/lib/ufsd.ko
	$(ROMFSINST) /etc_ro/ppp/3g/disable_pin.scr
