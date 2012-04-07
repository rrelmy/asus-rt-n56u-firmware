#
# milli_httpd Makefile
#
# Copyright (C) 2001 ASUSTeK Inc.
#
# $Id: Makefile,v 1.5 2007/04/14 11:19:47 shinjung Exp $
#

#.PHONY: all clean sym_link install romfs gen_syms

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS	+= -I. -I$(SHDIR)/include -I$(ROOTDIR)/linux-2.6.21.x/include -I../../linux-2.6.21.x/drivers/net/wireless/rt2860v2/include/ -I$(ROOTDIR)/linux-2.6.21.x/drivers/flash -I$(ROOTDIR)/linux-2.6.21.x/drivers/net/raeth -I$(ROOTDIR)/lib/include -I$(USERDIR)/libdisk -I$(SHDIR)

#2008.10 magic add -DWSC
CFLAGS	+= -s -Wall -DASUS_NVRAM -DASUS_DDNS -DTRANSLATE_ON_FLY -DFLASH_EMULATOR -DLinux -DMIPS -DWSC -DRalink_WPS $(GLOBAL_OPTIONS)
LDFLAGS	= -L$(SHDIR) -L$(USERDIR)/netconf -L$(USERDIR)/libdisk -ldisk

#2008.10 magic add detectWAN
CFLAGS  += -I../detectWAN
LDFLAGS += -L../detectWAN -ldetectWAN
LDFLAGS += -lm

#CFLAGS += -DW7_LOGO
#CFLAGS += -DWIFI_LOGO

vpath %.o $(ROOTDIR)/user/shared #2008.10 magic
#vpath %.c $(TOP)/shared
#vpath %.c $(TOP)/shared_common

ifeq ($(CONFIG_NOUSB),y)
CFLAGS += -DNOUSB
endif

ifeq ($(LANGUAGE),KR)
LANG_FILE = EN.dict KR.dict
else
LANG_FILE = EN.dict TW.dict CN.dict 
endif

#2008.10 magic{
#ifeq ($(MODEL_NAME), WL520GU)
#WWWDIR  = ../www/web_asus_en/WL520gu_text
WWWDIR = ../www/www_N56U  #Viz changed 2010.08 
#else
#WWWDIR = ../www/web_asus_en/WL500gp_text
#LANG_FILE += KR.dict
#endif

#vpath translate_%.txt $(WWWDIR)/
vpath %.dict $(WWWDIR)/
#2008.10 magic}

#all: httpd $(LANG_FILE)
all: httpd 
	$(STRIP) httpd

dep:
	@echo "Generate depend file..."
	@for i in `find . -name "*.c"`; do ${CC} -MM ${CFLAGS} "$${i}"; done > $@

clean:
	rm -f *.o *~ httpd
	for f in $(LANG_FILE) ; do rm -f $$f; done
	rm -f *.dict

#%.dict: translate_%.txt #2008.10 magic
%.dict: %.dict #2008.10 magic
	egrep '[^=]*=[^=]*' $^ | sed -e 's,<#ZVMODELVZ#>,$(MODEL_NAME),g' -e 's,\r,,' > $@

romfs:
	$(ROMFSINST) /usr/sbin/httpd

# 2008.10 magic. add notify_rc.o & discover.o & initial_web_hook.o
# Viz add aspbw.o 2010.08
httpd: httpd.o notify_rc.o discover.o ej.o cgi.o web_ex.o common.o nvram_f.o broadcom.o crc32.o initial_web_hook.o ../wireless_tools/iwlib.o aspbw.o
	echo $(LANGUAGE)
	#echo $(MODEL_NAME)
	$(CC) -o $@ $^ $(LDFLAGS) -lshared -lnetconf
	$(STRIP) $@

