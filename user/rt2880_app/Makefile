ra_dir_$(CONFIG_RALINKAPP_REG_RW)	+= reg
ra_dir_$(CONFIG_RALINKAPP_ATED)		+= ated
ra_dir_$(CONFIG_RALINKAPP_CSR)		+= csr_test
ra_dir_$(CONFIG_RALINKAPP_FLASH)	+= flash
ra_dir_$(CONFIG_RALINKAPP_HWNAT)	+= hw_nat
ra_dir_$(CONFIG_RALINKAPP_MII_MGR)	+= mii_mgr
ra_dir_$(CONFIG_RALINKAPP_NVRAM)	+= nvram
ra_dir_$(CONFIG_RALINKAPP_GPIO)		+= gpio
ra_dir_$(CONFIG_RALINKAPP_SPI)		+= spicmd
ra_dir_$(CONFIG_RALINKAPP_I2C)		+= i2ccmd
ra_dir_$(CONFIG_RALINKAPP_SCRIPTS)	+= scripts
ra_dir_$(CONFIG_RALINKAPP_MEMORY_USAGE)	+= mem_usage
ra_dir_$(CONFIG_RALINKAPP_SWITCH)	+= switch
ra_dir_$(CONFIG_RALINKAPP_I2S)		+= i2scmd
ra_dir_$(CONFIG_RALINKAPP_PCM)		+= pcmcmd
ra_dir_$(CONFIG_RALINKAPP_SWQOS)	+= sw_qos

ra_dir_y                                += libusb-1.0.6 # for madwimax
ra_dir_y                                += madwimax-0.1.1 # for Samsung WiMAX

PWD_DIR=$(shell pwd)

all romfs: config
	for i in $(ra_dir_y) ; do make -C $$i $@ || exit $?; done

config:
	make libusb10-config
	make madwimax-config

libusb10-config:
	cd $(PWD_DIR)/libusb-1.0.6 && \
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		./configure --prefix=$(PWD_DIR)/libusb-1.0.6/build --enable-shared --build=mipsel-linux; \
		rm -f config_done; \
		touch config_done; \
	fi )

madwimax-config:
	cd $(PWD_DIR)/madwimax-0.1.1 && \
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		./configure --prefix=$(PWD_DIR)/madwimax-0.1.1/build --with-script=wl500g --without-man-pages --with-udev-dir=$(PWD_DIR)/madwimax-0.1.1/build --build=mipsel-linux libusb1_CFLAGS=-I$(PWD_DIR)/libusb-1.0.6/libusb libusb1_LIBS="-L$(PWD_DIR)/libusb-1.0.6/libusb/.libs -lusb-1.0"; \
		rm -f config_done; \
		touch config_done; \
	fi )

configclean:
	make libusb10-configclean
	make madwimax-configclean

libusb10-configclean:
	cd $(PWD_DIR)/libusb-1.0.6 && rm -f config_done

madwimax-configclean:
	cd $(PWD_DIR)/madwimax-0.1.1 && rm -f config_done

clean:
	for i in $(ra_dir_y) ; do make -C $$i clean ; done
	make configclean
