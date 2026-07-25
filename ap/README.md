# AP stack (hostapd) for the RTL8832BR

The vendor g6 driver is cfg80211/nl80211-native; WEXT is legacy (the kernel warns
"use nl80211") and its scan ioctl never reaches this driver. AP mode is what a
router actually needs, so hostapd over nl80211 is the supported path.

Build (both sources ship in the vendor SDKs, no network needed):

    libnl-3.2.25   rtl8198d-sdk-main/lib/libnl/libnl-3.2.25
    hostapd-2.11   rtl8198d-sdk-main/user/hostapd/hostapd-2.11

1. libnl:  ./configure --host=$HOST --prefix=$STAGING --disable-cli \
             --disable-static --enable-shared && make && make install
2. hostapd: .config with CONFIG_DRIVER_NL80211=y, CONFIG_LIBNL32=y,
   CONFIG_IEEE80211AC/AX/N=y, CONFIG_TLS=internal,
   CONFIG_INTERNAL_LIBTOMMATH=y  (internal crypto avoids an openssl port)

   IMPORTANT: export PKG_CONFIG_LIBDIR=$STAGING/lib/pkgconfig before building,
   otherwise hostapd's pkg-config picks up the HOST's /usr/include/libnl3 and
   gcc rejects it ("unsafe header/library path used in cross-compilation").

Runtime: /sbin/hostapd + /lib/libnl-3.so.200 + /lib/libnl-genl-3.so.200.
Configs: /etc/hostapd.conf (WPA2-PSK) and /etc/hostapd-open.conf (open, for
first-light debugging). 5 GHz ch36 -- this radio is band_cap:5G only.
