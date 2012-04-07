#
# Broadcom web pages
#
# Copyright 2003, ASUSTeK Inc.
# All Rights Reserved.                
#                                     
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;   
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior      
# written permission of ASUSTeK Inc..                            
#
# $Id: Makefile,v 1.3 2007/04/09 12:08:09 shinjung Exp $
#

PAGES := *
PAGES_DIR := graph/*
LANGUAGE = EN
ROMFS_DIR = $(ROOTDIR)/romfs

#all: install-$(MODEL_NAME)
all:
#pages.mk

romfs:
	echo "Make www romfs"
	rm -Rf $(ROMFS_DIR)/www
#	cp -R www_N13 $(ROMFS_DIR)/www
	cp -R www_N56U $(ROMFS_DIR)/www
	rm -f $(ROMFS_DIR)/www/httpd_check.htm
	echo "ASUSTeK" > $(ROMFS_DIR)/www/httpd_check.htm
	touch $(ROMFS_DIR)/tmp/settings
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings.CFG && cd ../..
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings_RT-N56U.CFG && cd ../..
	rm -f $(ROMFS_DIR)/tmp/settings

#pages.mk:
#	echo "PAGES := $(PAGES)" > $@

install:
	./mkweb WL500gx "EN"	
	install -d $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.asp $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.js $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.css $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/graph
	install -m 644 web_WL500gx/$(PAGES_DIR).jpg $(INSTALLDIR)/www/graph
	install -m 644 web_WL500gx/$(PAGES_DIR).gif $(INSTALLDIR)/www/graph
	rm -rf web
	rm -rf web_WL500gx

install-%:
	echo "www install-"
	echo $(LANGUAGE)
	./mkweb $* "$(LANGUAGE)"
	rm -rf www_$*	
	install -d www_$*
	install -m 644 web_$*/*.asp www_$*
	install -m 644 web_$*/*.js www_$*
	install -m 644 web_$*/*.css www_$*
#	( if [ "$*" != "WL520g" ] && [ "$*" != "WL550gE" ] && [ "$*" != "WL500bl" ] && [ "$*" != "WL331gl" ]; then \
		install -m 644 web_$*/*.cab www_$*;\
	fi )
	install -d www_$*/graph
	install -m 644 web_$*/$(PAGES_DIR).jpg www_$*/graph
	install -m 644 web_$*/$(PAGES_DIR).gif www_$*/graph

	ln -sf /tmp/settings www_$*/Settings.CFG
	sleep 1
	#cp ../httpd/*.dict www_$*/ -f		# disable for tmp
	#rm -rf ../httpd/*.dict			# disable for tmp
	( if [ "$*" = "WL500gpv2" ]; then \
		install -d www_$*/slider; \
		install -m 644 web_$*/slider/*.js www_$*/slider; \
		install -m 644 web_$*/slider/*.gif www_$*/slider; \
	fi )

	ln -sf /var/tmp/display.jpg www_$*/display.jpg

	rm -rf web
	rm -rf web_$*

clean:
#	rm -f pages.mk
