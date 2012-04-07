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
		--with-ogg-libraries=$(LIB_HOME)/../../libogg-1.2.2/install/lib \
		--prefix=$(LIB_HOME)/../install \
	)

clean:
	$(MAKE) -C source clean
	rm -rf install
	rm -f config_done

romfs:
	echo "do nothing"
