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
		OGG_LIBS="-logg -L$(LIB_HOME)/../../libogg-1.2.2/install/lib" \
		--prefix=$(LIB_HOME)/../install \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f libvorbis.so.0.4.5
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libvorbis.so.0.4.5 .
	$(STRIP) libvorbis.so.0.4.5
	$(ROMFSINST) /lib/libvorbis.so.0.4.5
	cd $(INSTALLDIR)/lib && ln -sf libvorbis.so.0.4.5 libvorbis.so.0
	cd $(INSTALLDIR)/lib && ln -sf libvorbis.so.0.4.5 libvorbis.so

