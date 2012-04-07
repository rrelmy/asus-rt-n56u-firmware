MACHINE = mipsel
CFLAGS += -fno-builtin
export CFLAGS MACHINE

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

#all:
#	make configure
#	$(MAKE) -C source

configure: source/Makefile

source/Makefile:
	( cd source ; \
	export SMB_BUILD_CC_NEGATIVE_ENUM_VALUES=yes ; \
	export libreplace_cv_READDIR_GETDIRENTRIES=no ; \
	export libreplace_cv_READDIR_GETDENTS=no ; \
	export linux_getgrouplist_ok=no ; \
	export samba_cv_REPLACE_READDIR=no ; \
	export samba_cv_HAVE_WRFILE_KEYTAB=yes ; \
	export samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes ; \
	export samba_cv_HAVE_IFACE_IFCONF=yes ; \
	export samba_cv_USE_SETRESUID=yes ; \
	CC=mipsel-linux-gcc ./configure \
		--target=$(MACHINE)-linux \
		--host=$(MACHINE)-linux \
		--build=`/bin/arch`-linux \
		--enable-shared \
		--disable-static \
		--disable-cups \
		--disable-iprint \
		--disable-pie \
		--disable-fam \
		--bindir=/bin \
		--sbindir=/sbin \
		--libdir=/lib \
		--localstatedir=/var \
		--with-configdir=/etc \
		--with-privatedir=/etc \
		--with-rootsbindir=/sbin \
		--with-lockdir=/var/locks \
		--with-piddir=/var/locks \
		--without-ldap \
		--without-cifsmount \
		--without-sys-quotas \
		--prefix=/var; \
	)

clean:
#	( cd source; make -f Makefile.orig distclean )
	( cd source; make -f Makefile distclean )
	rm -f config_done
	rm -f smbd
	rm -f nmbd
#	rm -f smbcontrol
#	rm -f smbclient
	rm -f smbpasswd
#	rm -f net
#	rm -f tdbbackup
	rm -f libbigballofmud.so

distclean: clean
	( cd source; make -f Makefile.orig distclean )
	( find source -name config.h | xargs rm -f )
	#( find source -name Makefile | xargs rm -f )

romfs:
	echo "samba romfs"
	cp source/bin/smbd ./
	cp source/bin/nmbd ./
#	cp source/bin/smbcontrol ./
#	cp source/bin/smbclient ./
	cp source/bin/smbpasswd ./
#	cp source/bin/net ./
#	cp source/bin/tdbbackup ./
	cp source/bin/libbigballofmud.so ./
	$(STRIP) smbd
	$(STRIP) nmbd
#	$(STRIP) smbcontrol
#	$(STRIP) smbclient
	$(STRIP) smbpasswd
#	$(STRIP) net
#	$(STRIP) tdbbackup
	$(STRIP) libbigballofmud.so
	$(ROMFSINST) /sbin/smbd
	$(ROMFSINST) /sbin/nmbd
#	$(ROMFSINST) /usr/local/samba/bin/smbcontrol
#	$(ROMFSINST) /usr/local/samba/bin/smbclient
	$(ROMFSINST) /bin/smbpasswd
#	$(ROMFSINST) /usr/local/samba/bin/net
#	$(ROMFSINST) /usr/local/samba/bin/tdbbackup
	$(ROMFSINST) /lib/libbigballofmud.so
	cd $(INSTALLDIR)/lib && ln -sf libbigballofmud.so libbigballofmud.so.0
	rm -f smbd
	rm -f nmbd
#	rm -f smbcontrol
#	rm -f smbclient
	rm -f smbpasswd
#	rm -f net
#	rm -f tdbbackup
	rm -f libbigballofmud.so

install:
	rm -rf $(INSTALLDIR)
	$(MAKE) -C source install DESTDIR=$(INSTALLDIR)
	install -d $(INSTALLDIR)/lib/
	#install /opt/eldk/ppc_4xx/usr/lib/libpopt.so.0 $(INSTALLDIR)/lib/libpopt.so.0
	install source/bin/libbigballofmud.so $(INSTALLDIR)/lib/libbigballofmud.so.0
	rm -rf $(INSTALLDIR)/usr/local/samba/include/
	rm -rf $(INSTALLDIR)/usr/local/samba/share
	rm -rf $(INSTALLDIR)/usr/local/samba/swat
	rm -rf $(INSTALLDIR)/usr/local/samba/lib
	rm -rf $(INSTALLDIR)/tmp
	rm -f $(INSTALLDIR)/usr/local/samba/lib/*.msg
	rm -rf $(INSTALLDIR)/usr/local/samba/sbin/swat
	rm -rf $(INSTALLDIR)/usr/local/samba/sbin/winbindd
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/findsmb
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/nmblookup
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/rpcclient
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbcacls
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbspool
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbstatus
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbtar
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/testparm
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/wbinfo
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/ntlm_auth	
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/pdbedit	
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/profiles
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbcquotas
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbtree
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/tdbdump
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/smbget
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/eventlogadm
	rm -rf $(INSTALLDIR)/usr/local/samba/bin/tdbtool

	$(STRIP) --strip-all $(INSTALLDIR)/usr/local/samba/sbin/*
	$(STRIP) --strip-all $(INSTALLDIR)/usr/local/samba/bin/*

	install -d $(INSTALLDIR)/etc
	cd $(INSTALLDIR)/etc && ln -sf /tmp/passwd passwd
	cd $(INSTALLDIR)/etc && ln -sf /tmp/group group
