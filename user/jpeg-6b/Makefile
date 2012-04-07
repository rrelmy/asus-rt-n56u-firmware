JPEG_HOME = $(shell pwd)/source

all:	config_test
	mkdir -p install/include
	mkdir -p install/lib
	$(MAKE) -C source install-lib

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
		--enable-shared \
		--enable-static \
		--prefix=$(JPEG_HOME)/../install \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f libjpeg.so.62.0.0
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libjpeg.so.62.0.0 .
	$(STRIP) libjpeg.so.62.0.0
	$(ROMFSINST) /lib/libjpeg.so.62.0.0
	cd $(INSTALLDIR)/lib && ln -sf libjpeg.so.62.0.0 libjpeg.so.62
	cd $(INSTALLDIR)/lib && ln -sf libjpeg.so.62.0.0 libjpeg.so

