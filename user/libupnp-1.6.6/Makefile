LIBUPNP_HOME = $(shell pwd)/source

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
		--prefix=$(LIBUPNP_HOME)/../install \
		--without-PACKAGE \
		--without-documentation \
		--host=mipsel-linux; \
	)

clean:
	$(MAKE) -C source clean
	rm -rf install
	rm -f config_done

romfs:
	echo "do nothing"
