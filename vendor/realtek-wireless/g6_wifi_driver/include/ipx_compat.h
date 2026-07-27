/* IPX was removed from the kernel (net/ipx.h). rtw_br_ext.c's bridge helper
 * still parses IPX frames, so provide just the on-wire definitions it needs.
 * Layout per the historical kernel header / Novell IPX spec. */
#ifndef __RTW_IPX_COMPAT_H__
#define __RTW_IPX_COMPAT_H__
#include <linux/types.h>
#define IPX_NODE_LEN 6
struct ipx_address {
	__be32	net;
	__u8	node[IPX_NODE_LEN];
	__be16	sock;
};
struct ipxhdr {
	__be16			ipx_checksum __packed;
	__be16			ipx_pktsize __packed;
	__u8			ipx_tctrl;
	__u8			ipx_type;
	struct ipx_address	ipx_dest __packed;
	struct ipx_address	ipx_source __packed;
};
#endif
