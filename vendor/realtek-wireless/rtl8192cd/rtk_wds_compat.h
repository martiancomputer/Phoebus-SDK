/* PHOEBUS 6.18: struct rtk_wds_config, lifted verbatim from the vendor's
 * net80211/ieee80211_ioctl.h. Including that whole header here collides with
 * 8192cd_net80211.h (both define ieee80211req_key/_del_key/_mlme), so we take
 * only the one type this driver actually needs.
 */
#ifndef __RTK_WDS_COMPAT_H__
#define __RTK_WDS_COMPAT_H__
#ifndef MAX_WDS_NUM
#define MAX_WDS_NUM 4
#endif
#ifndef HOSTAPD_MAX_SSID_LEN
#define HOSTAPD_MAX_SSID_LEN 32
#endif

struct rtk_wds_config {
	int				wdsEnabled;
	unsigned char 	macAddr [MAX_WDS_NUM][6];
	int				wdsNum;
	char 			ssid[HOSTAPD_MAX_SSID_LEN + 1];
	int				wdsPrivacy;
	int				wdsWepKeyLen;
	char			*wdsWepKey;
	char			*wdsPskPassPhrase;
};
#endif
