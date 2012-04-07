LIBOGG_HOME = $(shell pwd)/source

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
		--prefix=$(LIBOGG_HOME)/../install \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f libogg.so.0.7.1
	rm -f config_done

romfs:
#	echo "do nothing"
	cp install/lib/libogg.so.0.7.1 .
	$(STRIP) libogg.so.0.7.1
	$(ROMFSINST) /lib/libogg.so.0.7.1
	cd $(INSTALLDIR)/lib && ln -sf libogg.so.0.7.1 libogg.so.0
	cd $(INSTALLDIR)/lib && ln -sf libogg.so.0.7.1 libogg.so

