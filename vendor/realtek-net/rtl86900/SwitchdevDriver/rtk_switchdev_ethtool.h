#ifndef _RTK_SWITCHDEV_ETHTOOL_H_
#define _RTK_SWITCHDEV_ETHTOOL_H_
#include "rtk_switchdev_gmac.h"

/* Statistics counters collected by the MAC */
struct rtksw_ethtool_stats {
	u64 rxucpktcnt;
	u64 rxmcfrmcnt;
	u64 rxbcfrmcnt;
	u64 rxoamfrmcnt;
	u64 rxjumbofrmcnt;
	u64 rxpausefrmcnt;
	u64 rxunknownocfrmcnt;
	u64 rxcrcerrfrmcnt;
	u64 rxundersizefrmcnt;
	u64 rxruntfrmcnt;
	u64 rxovsizefrmcnt;
	u64 rxjabberfrmcnt;
	u64 rxinvalidfrmcnt;
	u64 rxstatsfrm64oct;
	u64 rxstatsfrm65to127oct;
	u64 rxstatsfrm128to255oct;
	u64 rxstatsfrm256to511oct;
	u64 rxstatsfrm512to1023oct;
	u64 rxstatsfrm1024to1518oct;
	u64 rxstatsfrm1519to2100oct;
	u64 rxstatsfrm2101to9200oct;
	u64 rxstatsfrm9201tomaxoct;
	u64 rxbytecount_lo;
	u64 rxbytecount_hi;
	u64 txucpktcnt;
	u64 txmcfrmcnt;
	u64 txbcfrmcnt;
	u64 txoamfrmcnt;
	u64 txjumbofrmcnt;
	u64 txpausefrmcnt;
	u64 txcrcerrfrmcnt;
	u64 txovsizefrmcnt;
	u64 txsinglecolfrm;
	u64 txmulticolfrm;
	u64 txlatecolfrm;
	u64 txexesscolfrm;
	u64 txstatsfrm64oct;
	u64 txstatsfrm65to127oct;
	u64 txstatsfrm128to255oct;
	u64 txstatsfrm256to511oct;
	u64 txstatsfrm512to1023oct;
	u64 txstatsfrm1024to1518oct;
	u64 txstatsfrm1519to2100oct;
	u64 txstatsfrm2101to9200oct;
	u64 txstatsfrm9201tomaxoct;
	u64 txbytecount_lo;
	u64 txbytecount_hi;
};

/* number of ETHTOOL_GSTATS u64's */
#define RTKSW_NUM_STATS		(sizeof(struct rtksw_ethtool_stats)/sizeof(u64))

static const struct {
	const char string[ETH_GSTRING_LEN];
} rtksw_ethtool_stats_keys[RTKSW_NUM_STATS] = {
	{ "RxUCPktCnt" },
	{ "RxMCFrmCnt" },
	{ "RxBCFrmCnt" },
	{ "RxOAMFrmCnt" },
	{ "RxJumboFrmCnt" },
	{ "RxPauseFrmCnt" },
	{ "RxUnKnownOCFrmCnt" },
	{ "RxCrcErrFrmCnt" },
	{ "RxUndersizeFrmCnt" },
	{ "RxRuntFrmCnt" },
	{ "RxOvSizeFrmCnt" },
	{ "RxJabberFrmCnt" },
	{ "RxInvalidFrmCnt" },
	{ "RxStatsFrm64oct" },
	{ "RxStatsFrm65to127oct" },
	{ "RxStatsFrm128to255oct" },
	{ "RxStatsFrm256to511oct" },
	{ "RxStatsFrm512to1023oct" },
	{ "RxStatsFrm1024to1518oct" },
	{ "RxStatsFrm1519to2100oct" },
	{ "RxStatsFrm2101to9200oct" },
	{ "RxStatsFrm9201toMaxoct" },
	{ "RxByteCount_Lo" },
	{ "RxByteCount_Hi" },
	{ "TxUCPktCnt" },
	{ "TxMCFrmCnt" },
	{ "TxBCFrmCnt" },
	{ "TxOAMFrmCnt" },
	{ "TxJumboFrmCnt" },
	{ "TxPauseFrmCnt" },
	{ "TxCrcErrFrmCnt" },
	{ "TxOvSizeFrmCnt" },
	{ "TxSingleColFrm" },
	{ "TxMultiColFrm" },
	{ "TxLateColFrm" },
	{ "TxExessColFrm" },
	{ "TxStatsFrm64oct" },
	{ "TxStatsFrm65to127oct" },
	{ "TxStatsFrm128to255oct" },
	{ "TxStatsFrm256to511oct" },
	{ "TxStatsFrm512to1023oct" },
	{ "TxStatsFrm1024to1518oct" },
	{ "TxStatsFrm1519to2100oct" },
	{ "TxStatsFrm2101to9200oct" },
	{ "TxStatsFrm9201toMaxoct" },
	{ "TxByteCount_Lo" },
	{ "TxByteCount_Hi" }
};

void rtksw_set_ethtool_ops(struct net_device *dev);

#endif /* _RTK_SWITCHDEV_ETHTOOL_H_ */