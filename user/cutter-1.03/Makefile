CFLAGS += -O2 -Wall -g

cutter: cutter.c
	$(CC) cutter.c -o cutter

romfs:
	$(STRIP) cutter
	$(ROMFSINST) /usr/bin/cutter

clean:
	rm -f cutter.o cutter
