LIB_HOME = $(shell pwd)/source

all:	config_test
	$(MAKE) -C source install

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure; \
		rm -f config_done; \
		touch config_done; \
	fi ) 

configure:
	( cd source ; \
	./configure \
		--host=mipsel-linux \
		--enable-sqlite3 \
		--with-sqlite3-includes=$(LIB_HOME)/../../sqlite-autoconf-3070500/install/include \
		--with-sqlite3-libs=$(LIB_HOME)/../../sqlite-autoconf-3070500/install/lib \
		--with-id3tag=$(LIB_HOME)/../../../lib \
		--prefix=$(LIB_HOME)/../install \
	)

clean:
	( if [ -f ./source/Makefile ]; then \
		$(MAKE) -C source distclean; \
	fi ) 
	rm -rf install
	rm -rf *.so
	rm -f mt-daapd
	rm -f config_done

romfs:
	cp install/sbin/mt-daapd .
	$(STRIP) mt-daapd
	$(ROMFSINST) /usr/sbin/mt-daapd
	cp install/lib/mt-daapd/plugins/*.so .
	$(STRIP) *.so
	$(ROMFSINST) /etc_ro/mt-daapd/plugins/out-daap.so
	$(ROMFSINST) /etc_ro/mt-daapd/plugins/rsp.so
	$(ROMFSINST) /etc_ro/mt-daapd/plugins/ssc-script.so
	cp -r install/share/mt-daapd/admin-root $(ROOTDIR)/romfs/etc_ro/mt-daapd
