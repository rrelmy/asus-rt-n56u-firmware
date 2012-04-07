SQLITE_HOME = $(shell pwd)/source

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
		--enable-static \
		--prefix=$(SQLITE_HOME)/../install \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f libsqlite3.so.0.8.6
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libsqlite3.so.0.8.6 .
	$(STRIP) libsqlite3.so.0.8.6
	$(ROMFSINST) /lib/libsqlite3.so.0.8.6
	cd $(INSTALLDIR)/lib && ln -sf libsqlite3.so.0.8.6 libsqlite3.so.0
	cd $(INSTALLDIR)/lib && ln -sf libsqlite3.so.0.8.6 libsqlite3.so

