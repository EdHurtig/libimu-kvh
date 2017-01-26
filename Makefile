OBJECTS= imu.o crc.o
LDFLAGS += -L.
LDLIBS += -limu
DSTROOT= ./BUILD/dst

all: $(OBJECTS)
	ar rcs libimu.a $(OBJECTS)

test: test.o

config: config.o

debug: all test config

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
	rm -rf BUILD test config *.o *.a
