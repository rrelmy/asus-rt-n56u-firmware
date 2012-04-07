LIBEXIF_HOME = $(shell pwd)/source

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
		--disable-nls \
		--disable-docs \
		--prefix=$(LIBEXIF_HOME)/../install \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f libexif.so.12.3.2
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libexif.so.12.3.2 .
	$(STRIP) libexif.so.12.3.2
	$(ROMFSINST) /lib/libexif.so.12.3.2
	cd $(INSTALLDIR)/lib && ln -sf libexif.so.12.3.2 libexif.so.12
	cd $(INSTALLDIR)/lib && ln -sf libexif.so.12.3.2 libiconv.so

