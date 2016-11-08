OBJECTS= imu.o crc.o
LDFLAGS += -lz
DSTROOT= ./BUILD/dst

all: $(OBJECTS)
	ar rcs libimu.a $(OBJECTS)

test: all test.o
	$(CC) -limu -L. test.o -o test

config: all config.o
	$(CC) -limu -L. config.o -o config

debug: test config all

install:
	mkdir -p $(DSTROOT)/usr/local/lib
	mkdir -p $(DSTROOT)/usr/local/include
	mkdir -p $(DSTROOT)/usr/local/bin
	install -m 755 ./libimu.a $(DSTROOT)/usr/local/lib
	install -m 755 ./imu.h $(DSTROOT)/usr/local/include

install-debug: install
	install -m 755 ./test $(DSTROOT)/usr/local/bin
	install -m 755 ./config $(DSTROOT)/usr/local/bin

clean:
	rm -rf BUILD test config *.o
