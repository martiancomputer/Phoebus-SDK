#ifndef _RTL_GENERIC_NETLINK_H
#define _RTL_GENERIC_NETLINK_H
#include <linux/genetlink.h>

#ifndef __KERNEL__
/* ========================================
 * Common Generic Netlink API
 ======================================== */
#define GENLMSG_DATA(glh)       ((void*)(((char*)glh) + GENL_HDRLEN))
#define NLA_DATA(nla)           ((void *)((char*)(nla) + NLA_HDRLEN))
#define NLA_NEXT(nla,len)       ((len) -= NLA_ALIGN((nla)->nla_len), \
                                      (struct nlattr*)(((char*)(nla)) + NLA_ALIGN((nla)->nla_len)))
#define NLA_OK(nla,len)         ((len) >= (int)sizeof(struct nlattr) && \
                                    (nla)->nla_len >= sizeof(struct nlattr) && \
                                    (nla)->nla_len <= (len))

/* nlmag payload include
 *	  1. nlmsg header
 *	  2. genlmsg header
 *	  3. user define header (optional)
 *		  Hint: it depends on hdrsize of genl_family, you can use genl-ctrl-list tool to check it.
 *	  4. nlattr header
 *	  5. real payload length
 **/
#define GENLMSG_TOTAL_LEN(family_hdr, len) NLMSG_SPACE(GENL_HDRLEN+(family_hdr)+rtk_nla_total_size(len))
#endif

/* ========================================
 * RTL Generic Netlink Customize Definition
 ======================================== */
/* Generic Netlink Family Name */

#define RTL_GENL_WLAN_EVENT	"WIFI6_INDICATE"
#define RTL_GENL_WLAN_EVENT_GROUP	"WIFI6_EVENT_GRP"

enum {
  RTL_WLAN_INDICATE_ATTR_UNSPEC,
  RTL_WLAN_INDICATE_ATTR_MSG,
  __RTL_WLAN_INDICATE_ATTR_MAX,
};
#define RTL_WLAN_INDICATE_ATTR_MAX (__RTL_WLAN_INDICATE_ATTR_MAX - 1)

enum {
  RTL_WLAN_INDICATE_CMD_UNSPEC,
  RTL_WLAN_INDICATE_CMD_EVENT,
  __RTL_WLAN_INDICATE_CMD_MAX,
};
#define RTL_WLAN_INDICATE_CMD_MAX (__RTL_WLAN_INDICATE_CMD_MAX - 1)
#endif	// _RTL_GENERIC_NETLINK_H
