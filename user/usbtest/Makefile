
BIN = usbtest

all: ${BIN}
	${STRIP} ${BIN}

usbtest: test.c
	$(CC) -o usbtest test.c

romfs:
	$(ROMFSINST) /usr/sbin/$(BIN)

clean:
	-rm -f usbtest
