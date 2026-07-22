#ifndef RTL_ATM_H
#define RTL_ATM_H

/*--- vendor-specific for atmarp.h ---*/
#define ATMARP_IP	_IO('a',ATMIOC_CLIP+6)	/* change ip */
#define ATMARP_SRV	_IO('a',ATMIOC_CLIP+7)	/* arp server ip */
#define ATMARP_INARPREP	_IO('a',ATMIOC_CLIP+8)	/* send InATMArpReply */

/* RFC 1577 ATM ARP header */

struct atmarphdr {
	uint16_t ar_hrd;	/* Hardware type */
	uint16_t ar_pro;	/* Protocol type */
	uint8_t ar_shtl;	/* Type & length of source ATM number (q) */
	uint8_t ar_sstl;	/* Type & length of source ATM subaddress (r) */
	uint16_t ar_op;	/* Operation code (request, reply, or NAK) */
	uint8_t ar_spln;	/* Length of source protocol address (s) */
	uint8_t ar_thtl;	/* Type & length of target ATM number (x) */
	uint8_t ar_tstl;	/* Type & length of target ATM subaddress (y) */
	uint8_t ar_tpln;	/* Length of target protocol address (z) */
	/* ar_sha, at_ssa, ar_spa, ar_tha, ar_tsa, ar_tpa */
	unsigned char data[1];
};

#define TL_LEN 0x3f	/* ATMARP Type/Length field structure */

/*--- vendor-specific for atmdev.h ---*/
#define ATM_SETBRIDGEPPPOE _IOW('a',ATMIOC_SPECIAL+6,atm_backend_t)
#define ATM_SETVLAN     _IOW('a',ATMIOC_SPECIAL+7,atm_backend_t)
                                        /* set VLAN mapping */
#define ATM_SETITFGRP   _IOW('a',ATMIOC_SPECIAL+8,atm_backend_t)
                                        /* set interface group */
#define ATM_SETFGROUP _IOW('a',ATMIOC_SPECIAL+9,atm_backend_t)

/*--- vendor-specific for atmbr2684.h ---*/
struct vlan {
	int	vlan;		/* vlan flag */
	int	vid;		/* vlan tag */
	int	vlan_prio;	/* vlan priority bits */
	int	vlan_pass;	/* vlan passthrough */
};

struct itfgrp {
	int flag;		/* group flag */
	int member;		/* bit-mapped LAN interface member */
};

/*
 * This is for the vendor-specific ATM_SETBACKEND call - these are like socket families:
 */
#include <linux/atmbr2684.h>
struct atm_backend_vendor {
	atm_backend_t backend_num;	/* ATM_BACKEND_BR2684 */
	struct br2684_if_spec ifspec;
	struct vlan vlan_tag;
	struct itfgrp ifgrp;	/* interface group */
	uint16_t fgroup;	/* portmapping */
};

#endif

