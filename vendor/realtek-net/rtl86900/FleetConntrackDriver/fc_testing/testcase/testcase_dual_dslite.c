#include <test_define.h>
#include <pktGen.h>
#include <rtl_util.h>
#include <rtl_glue.h>
#include <dump.h>


int dslite_upstream(rtk_fcTesting_case_t *info)
{
	int igr_ctagif=0,igr_cvid=WAN_CVLAN_0,igr_cpri=WAN_CPRI_0;
	int igr_stagif=0,igr_svid=WAN_SVLAN_0,igr_spri=WAN_SPRI_0;
	uint8 igr_pppoeTagif=0,igr_pppoeSID=SESSION_ID_11;
	uint16 igr_sport=0x1111,igr_dport=0x2222;
	uint8 igr_Pktsmac[6],igr_Pktdmac[6];
	uint32 igr_pktsip,igr_pktdip;
	//uint8 igr_pktv6sip[16],igr_pktv6dip[16];
	uint8 igr_ttl=10;


	int egr_ctagif=1,egr_cvid=WAN_CVLAN_0,egr_cpri=WAN_CPRI_0;
	int egr_stagif=0,egr_svid=WAN_SVLAN_0,egr_spri=WAN_SPRI_0;
	uint8 egr_pppoeTagif=0,pppoeSID=SESSION_ID_11;
	uint16 egr_sport=0x1111,egr_dport=0x2222;
	uint8 egr_pktv6sip[16],egr_pktv6dip[16];
	uint8 egr_Pktsmac[6],egr_Pktdmac[6];
	uint32 egr_pktsip,egr_pktdip;
	uint8 egr_ttl=igr_ttl-1;

	TEST_DEBUG("dslite dslite_upstream ");
	{
		strtomac((void *)&igr_Pktsmac, LAN_HOST_MAC_0);
		strtomac((void *)&igr_Pktdmac, LAN_GW_MAC_0);	
		strtoip(&igr_pktsip,LAN_HOST_IP4_0);
		strtoip(&igr_pktdip,REMOTE_HOST_IP4_0);

		v4TcpUdpPktGen2
		(
			info->igrInfo.igrData/*packet buf*/,&igrPayload[0]/*packet content*/,&info->igrInfo.igrDataLen/*pktLen*/,_PKT_TYPE_UDP/*packetType*/,
			igr_Pktsmac,igr_Pktdmac,
			igr_pktsip,igr_pktdip,
			igr_sport,igr_dport,
			igr_ttl/*ip_ttl*/,0/*DSCP*/,
			igr_ctagif/*Ctagif*/,igr_cvid/*CVID*/,igr_cpri/*CPRI*/,
			igr_stagif/*Stagif*/,0x88a8/*tpid*/,igr_svid/*SVID*/,igr_spri/*SPRI*/,
			igr_pppoeTagif/*pppoeTagif*/,igr_pppoeSID/*PPPoESSID*/
		);

		info->igrInfo.igrPort=RTK_FC_MAC_PORT1;
		//test_dump_packet(info->igrInfo.igrData,info->igrInfo.igrDataLen,"IPv4 Pkt");
		
	}



	{
		in6_pton(DSLITE_LOCAL_B4v6IP ,-1,(&egr_pktv6sip[0]),-1,NULL);
		in6_pton(DSLITE_REMOTE_AFTR_V6IP ,-1,(&egr_pktv6dip[0]),-1,NULL); 
		strtomac((void *)&egr_Pktsmac[0], WAN_NAS0_0_MAC);
		strtomac((void *)&egr_Pktdmac[0], REMOTE_HOST_MAC_0);	
		strtoip(&egr_pktsip,LAN_HOST_IP4_0);
		strtoip(&egr_pktdip,REMOTE_HOST_IP4_0);


		dslitePktGenByPatten(info->egrInfo.egrData, &egrPayload[0], &info->egrInfo.egrDataLen, _PKT_TYPE_UDP, 0 /*tcpFlag*/,
				egr_Pktsmac, egr_Pktdmac,
				egr_pktv6sip, egr_pktv6dip,
				egr_pktsip, egr_pktdip,
				egr_sport, egr_dport,
				egr_ctagif,egr_cvid,egr_cpri,
				egr_stagif,egr_svid,egr_spri, 
				egr_pppoeTagif,pppoeSID, 
				egr_ttl/*ttl*/, 10/*hoplimit*/,	0/*mf*/, 0/*tos*/ ,0/*tc*/);
		info->egrInfo.egrPort=RTK_FC_MAC_PORT_PON;
	}


	
	return SUCCESS;
}



int dslite_downstream_dualInLan(rtk_fcTesting_case_t *info)
{
	int igr_ctagif=1,igr_cvid=WAN_CVLAN_0,igr_cpri=WAN_CPRI_0;
	int igr_stagif=0,igr_svid=WAN_SVLAN_0,igr_spri=WAN_SPRI_0;
	uint8 igr_pppoeTagif=0,igr_pppoeSID=SESSION_ID_11;
	uint16 igr_sport=0x1111,igr_dport=0x2222;
	uint8 igr_Pktsmac[6],igr_Pktdmac[6];
	uint32 igr_pktsip,igr_pktdip;
	//uint8 igr_pktv6sip[16],igr_pktv6dip[16];	
	uint8 igr_ttl=10;

	int egr_ctagif=0,egr_cvid=WAN_CVLAN_0,egr_cpri=WAN_CPRI_0;
	int egr_stagif=0,egr_svid=WAN_SVLAN_0,egr_spri=WAN_SPRI_0;
	uint8 egr_pppoeTagif=0,pppoeSID=SESSION_ID_11;
	uint16 egr_sport=0x1111,egr_dport=0x2222;
	uint8 egr_pktv6sip[16],egr_pktv6dip[16];
	uint8 egr_Pktsmac[6],egr_Pktdmac[6];
	uint32 egr_pktsip,egr_pktdip;
	uint8 egr_ttl=igr_ttl-1;

	TEST_DEBUG("dslite dslite_downstream_dualInLan ");
	{
		strtomac((void *)&igr_Pktsmac, REMOTE_HOST_MAC_0);
		strtomac((void *)&igr_Pktdmac, WAN_NAS0_0_MAC);	
		strtoip(&igr_pktsip,REMOTE_HOST_IP4_0 );
		strtoip(&igr_pktdip,LAN_HOST_IP4_0);

		v4TcpUdpPktGen2
		(
			info->igrInfo.igrData/*packet buf*/,&igrPayload[0]/*packet content*/,&info->igrInfo.igrDataLen/*pktLen*/,_PKT_TYPE_UDP/*packetType*/,
			igr_Pktsmac,igr_Pktdmac,
			igr_pktsip,igr_pktdip,
			igr_sport,igr_dport,
			igr_ttl/*ip_ttl*/,0/*DSCP*/,
			igr_ctagif/*Ctagif*/,igr_cvid/*CVID*/,igr_cpri/*CPRI*/,
			igr_stagif/*Stagif*/,0x88a8/*tpid*/,igr_svid/*SVID*/,igr_spri/*SPRI*/,
			igr_pppoeTagif/*pppoeTagif*/,igr_pppoeSID/*PPPoESSID*/
		);

		info->igrInfo.igrPort=RTK_FC_MAC_PORT_PON;
		//test_dump_packet(info->igrInfo.igrData,info->igrInfo.igrDataLen,"IPv4 Pkt");
		
	}



	{
		in6_pton(DSLITE_LOCAL_B4v6IP ,-1,(&egr_pktv6sip[0]),-1,NULL);
		in6_pton(DSLITE_REMOTE_AFTR_V6IP ,-1,(&egr_pktv6dip[0]),-1,NULL); 
		strtomac((void *)&egr_Pktsmac[0], LAN_GW_MAC_0);
		strtomac((void *)&egr_Pktdmac[0], LAN_HOST_MAC_0);
		strtoip(&egr_pktsip,REMOTE_HOST_IP4_0);
		strtoip(&egr_pktdip,LAN_HOST_IP4_0);


		dslitePktGenByPatten(info->egrInfo.egrData, &egrPayload[0], &info->egrInfo.egrDataLen, _PKT_TYPE_UDP, 0 /*tcpFlag*/,
				egr_Pktsmac, egr_Pktdmac,
				egr_pktv6sip, egr_pktv6dip,
				egr_pktsip, egr_pktdip,
				egr_sport, egr_dport,
				egr_ctagif,egr_cvid,egr_cpri,
				egr_stagif,egr_svid,egr_spri, 
				egr_pppoeTagif,pppoeSID, 
				egr_ttl/*ttl*/, 10/*hoplimit*/,	0/*mf*/, 0/*tos*/ ,0/*tc*/);
		info->egrInfo.egrPort=RTK_FC_MAC_PORT1;
	}


	
	return SUCCESS;
}


int dslite_upstream_dualInLan(rtk_fcTesting_case_t *info)
{
	/* igress */
	int igr_ctagif=0,igr_cvid=WAN_CVLAN_0,igr_cpri=WAN_CPRI_0;
	int igr_stagif=0,igr_svid=WAN_SVLAN_0,igr_spri=WAN_SPRI_0;
	uint8 igr_pppoeTagif=0,igr_pppoeSID=SESSION_ID_11;
	uint16 igr_sport=0x1111,igr_dport=0x2222;
	uint8 igr_Pktsmac[6],igr_Pktdmac[6];
	uint32 igr_pktsip,igr_pktdip;
	uint8 igr_pktv6sip[16],igr_pktv6dip[16];
	uint8 igr_ttl=10;

	/* egress */
	int egr_ctagif=1,egr_cvid=WAN_CVLAN_0,egr_cpri=WAN_CPRI_0;
	int egr_stagif=0,egr_svid=WAN_SVLAN_0,egr_spri=WAN_SPRI_0;
	uint8 egr_pppoeTagif=0,pppoeSID=SESSION_ID_11;
	uint16 egr_sport=0x1111,egr_dport=0x2222;
	//uint8 egr_pktv6sip[16],egr_pktv6dip[16];
	uint8 egr_Pktsmac[6],egr_Pktdmac[6];
	uint32 egr_pktsip,egr_pktdip;
	uint8 egr_ttl=igr_ttl-1;

	
	TEST_DEBUG("dslite dslite_upstream_dualInLan ");
	{


		in6_pton(DSLITE_REMOTE_AFTR_V6IP ,-1,(&igr_pktv6sip[0]),-1,NULL);
		in6_pton(DSLITE_LOCAL_B4v6IP,-1,(&igr_pktv6dip[0]),-1,NULL); 
		strtomac((void *)&igr_Pktsmac[0], LAN_HOST_MAC_0);
		strtomac((void *)&igr_Pktdmac[0], LAN_GW_MAC_0);
		strtoip(&igr_pktsip,LAN_HOST_IP4_0);
		strtoip(&igr_pktdip,REMOTE_HOST_IP4_0);		


		dslitePktGenByPatten(info->igrInfo.igrData, &igrPayload[0], &info->igrInfo.igrDataLen, _PKT_TYPE_UDP, 0 /*tcpFlag*/,
				igr_Pktsmac, igr_Pktdmac,
				igr_pktv6sip, igr_pktv6dip,
				igr_pktsip, igr_pktdip,
				igr_sport, igr_dport,
				igr_ctagif,igr_cvid,igr_cpri,
				igr_stagif,igr_svid,igr_spri, 
				igr_pppoeTagif,igr_pppoeSID, 
				igr_ttl/*ttl*/, 10/*hoplimit*/,	0/*mf*/, 0/*tos*/ ,0/*tc*/);
		info->igrInfo.igrPort=RTK_FC_MAC_PORT1;
		test_dump_packet(info->igrInfo.igrData,info->igrInfo.igrDataLen,"Dslite Pkt");


	}


	{

		strtomac((void *)&egr_Pktsmac[0], WAN_NAS0_0_MAC);
		strtomac((void *)&egr_Pktdmac[0], REMOTE_HOST_MAC_0);
		strtoip(&egr_pktsip,LAN_HOST_IP4_0);
		strtoip(&egr_pktdip,REMOTE_HOST_IP4_0);

		v4TcpUdpPktGen2
		(
			info->egrInfo.egrData/*packet buf*/,&egrPayload[0]/*packet content*/,&info->egrInfo.egrDataLen/*pktLen*/,_PKT_TYPE_UDP/*packetType*/,
			egr_Pktsmac,egr_Pktdmac,
			egr_pktsip,egr_pktdip,
			egr_sport,egr_dport,
			egr_ttl/*ip_ttl*/,0/*DSCP*/,
			egr_ctagif/*Ctagif*/,egr_cvid/*CVID*/,egr_cpri/*CPRI*/,
			egr_stagif/*Stagif*/,0x88a8/*tpid*/,egr_svid/*SVID*/,egr_spri/*SPRI*/,
			egr_pppoeTagif/*pppoeTagif*/,pppoeSID/*PPPoESSID*/
		);
		info->egrInfo.egrPort=RTK_FC_MAC_PORT_PON;
		//test_dump_packet(info->igrInfo.igrData,info->igrInfo.igrDataLen,"IPv4 Pkt");
	}


	
	return SUCCESS;
}


int gre_downstream_dualInLan(rtk_fcTesting_case_t *info)
{
	/*DA 00:e0:4c:86:70:02 */
	uint8 igress[]=
	{
		0x00,0xe0,0x4c,0x86,0x70,0x02,0xe4,0x5f,0x01,0x41,0xa6,0xb6,0x08,0x00,0x45,0x00,
		0x00,0x54,0x00,0x00,0x00,0x00,0x74,0x01,0xad,0xe6,0x08,0x08,0x08,0x08,0xc0,0xa8,
		0xc8,0x0a,0x00,0x00,0xc7,0xb8,0x03,0xbc,0x06,0xa5,0x7a,0x23,0xb3,0xc2,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00
	};

	uint8 egress[]=
	{
		0xc0,0x9f,0x51,0x8f,0x83,0x03,0x72,0xe0,0x4c,0x89,0x70,0x80,0x08,0x00,0x45,0x00,
		0x00,0x7a,0xd2,0xc4,0x40,0x00,0x73,0x2f,0xdb,0x8f,0xa9,0xfe,0x02,0x81,0xa9,0xfe,
		0x02,0x83,0x00,0x00,0x65,0x58,0xc2,0x9f,0x51,0x8f,0x83,0x00,0x00,0xe0,0x4c,0x86,
		0x70,0x02,0x08,0x00,0x45,0x00,0x00,0x54,0x00,0x00,0x00,0x00,0x73,0x01,0x4e,0x03,
		0x08,0x08,0x08,0x08,0xc0,0xa8,0x28,0xee,0x00,0x00,0xc7,0xb8,0x03,0xbc,0x06,0xa5,
		0x7a,0x23,0xb3,0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	memcpy(info->igrInfo.igrData,igress,sizeof(igress));
	info->igrInfo.igrDataLen = sizeof(igress);
	info->igrInfo.igrPort=RTK_FC_MAC_PORT_PON;

	memcpy(info->egrInfo.egrData,egress,sizeof(egress));
	info->egrInfo.egrDataLen = sizeof(egress);
	info->egrInfo.egrPort=RTK_FC_MAC_PORT1;
	info->egrInfo.igrLogicalDev[0] =__dev_get_by_name(&init_net,"nas0_0");
	info->egrInfo.egrLogicalDev[0] =__dev_get_by_name(&init_net,"eth0.2.0");

	
	return SUCCESS;

}

