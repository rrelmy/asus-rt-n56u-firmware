JPEG_HOME = $(shell pwd)/source

all:	config_test
#	mkdir -p install/include
#	mkdir -p install/lib
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
		--prefix=$(JPEG_HOME)/../install \
	)

clean:
	$(MAKE) -C source clean
	rm -rf install
	rm -f libjpeg.so.8.3.0
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libjpeg.so.8.3.0 .
	$(STRIP) libjpeg.so.8.3.0
	$(ROMFSINST) /lib/libjpeg.so.8.3.0
	cd $(INSTALLDIR)/lib && ln -sf libjpeg.so.8.3.0 libjpeg.so.8
	cd $(INSTALLDIR)/lib && ln -sf libjpeg.so.8.3.0 libjpeg.so

