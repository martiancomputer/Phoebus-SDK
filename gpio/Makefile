CC ?= cc
STRIP ?= strip
CFLAGS ?= -Os
CFLAGS += -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -fstack-protector-strong -fPIE
LDFLAGS += -pie -Wl,-z,relro,-z,now,-z,noexecstack
all: gpio-daemon gpioctl
gpio-daemon: daemon.c config.c config.h protocol.h
	$(CC) $(CPPFLAGS) $(CFLAGS) daemon.c config.c $(LDFLAGS) -o $@
gpioctl: gpioctl.c protocol.h
	$(CC) $(CPPFLAGS) $(CFLAGS) gpioctl.c $(LDFLAGS) -o $@
install: all
	install -d $(DESTDIR)/usr/sbin
	install -m 0755 gpio-daemon gpioctl $(DESTDIR)/usr/sbin/
	$(STRIP) $(DESTDIR)/usr/sbin/gpio-daemon $(DESTDIR)/usr/sbin/gpioctl
	ln -sf gpioctl $(DESTDIR)/usr/sbin/gpio-lease-notify
clean:
	rm -f gpio-daemon gpioctl
.PHONY: all install clean gpio-daemon gpioctl
