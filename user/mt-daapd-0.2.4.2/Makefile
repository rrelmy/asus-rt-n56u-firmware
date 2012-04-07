all:
	$(MAKE) -C src

romfs: all
	$(MAKE) -C src romfs
	$(ROMFSINST) mt-daapd.sh /bin
	cp -r admin-root $(ROOTDIR)/romfs/etc_ro/web

clean:
	$(MAKE) -C src clean
