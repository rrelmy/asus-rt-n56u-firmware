all:	config_test
	$(MAKE) -C source

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
		--disable-testio-debug \
		--disable-debugfs \
		--disable-imager \
		--disable-resizer \
		--enable-fsck \
		--disable-tls \
		--host=mipsel-linux; \
	)

clean:
	$(MAKE) -C source clean
	rm -f e2fsck
	rm -f config_done

romfs:
	cp source/e2fsck/e2fsck .
	$(STRIP) e2fsck
	$(ROMFSINST) /sbin/e2fsck

