#ifndef __RTK_FC_NIC_EXT_H__
#define __RTK_FC_NIC_EXT_H__

#if defined(CONFIG_RTK_L34_G3_PLATFORM)

#define RTK_FC_NIC_TX_BUSY_CARE 1	//for nic/fc function version control (care nic return busy)

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_WIFI_FF_DEVID(nh_priv) (nh_priv->wifi_sw_id)
#define RTK_FC_IS_WIFI_FF_UC(nh_priv) (nh_priv->isWifiFF)
#define RTK_FC_WIFI_FF_COS(nh_priv) (nh_priv->cos)

#else
#define RTK_FC_WIFI_FF_DEVID(nh_priv) (nh_priv->hdr_cpu->mdata_raw.mdata_l&0xffff)
#define RTK_FC_IS_WIFI_FF_UC(nh_priv) ((nh_priv->hdr_cpu->mdata_raw.mdata_l&0xffff) != 0)
#define RTK_FC_WIFI_FF_COS(nh_priv) (nh_priv->hdr_a->bits.cos)
#endif

#if defined(CONFIG_RTK_FC_PER_HW_FLOW_MIB) || defined(CONFIG_ARK_QOS)
#define RTK_FC_NH_HDR_A_EXIST(nh_priv)			(nh_priv->hdr_a)
#define RTK_FC_NH_HDR_A_LSPID(nh_priv)			(nh_priv->hdr_a->bits.lspid)

#define RTK_FC_NH_HDR_CPU_EXIST(nh_priv)		(nh_priv->hdr_cpu)
#define RTK_FC_NH_HDR_CPU_HASH_CRC16(nh_priv)		(nh_priv->hdr_cpu->l3fe_info.hash_crc16)
#define RTK_FC_NH_HDR_CPU_HASH_CRC32(nh_priv)		(nh_priv->hdr_cpu->l3fe_info.hash_crc32)
#endif
#endif

#endif
