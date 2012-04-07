FFMPEG_HOME = $(shell pwd)/source

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
		--cross-prefix=mipsel-linux- \
		--enable-cross-compile \
		--arch=mips \
		--target-os=linux \
		--prefix=$(FFMPEG_HOME)/../install \
		--enable-pthreads \
		--disable-debug \
		--disable-avdevice \
		--disable-swscale \
		--disable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-ffserver \
		--disable-swscale-alpha \
		--disable-encoders \
		--disable-decoders \
		--disable-bsfs \
		--disable-indevs \
		--disable-outdevs \
		--disable-devices \
		--disable-filters \
		--disable-muxers \
		--disable-parsers \
		--disable-protocols \
		--enable-protocol=file \
		--disable-aandct \
		--disable-dct \
		--disable-fft \
		--disable-golomb \
		--disable-lpc \
		--disable-mdct \
		--disable-rdft \
		--disable-vaapi \
		--disable-vdpau \
		--disable-dxva2 \
	)

clean:
	$(MAKE) -C source distclean
	rm -rf install
	rm -f config_done

romfs:
	echo "do nothing"
