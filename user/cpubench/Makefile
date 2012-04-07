CFLAGS += -g -O2

all: cpubench

cpubench: openwrt_cpu_bench_v06.o
	$(CC) openwrt_cpu_bench_v06.o -o cpubench

clean:
	rm -f cpubench *~ *.o

romfs:
	$(STRIP) cpubench
	$(ROMFSINST) /usr/sbin/cpubench
