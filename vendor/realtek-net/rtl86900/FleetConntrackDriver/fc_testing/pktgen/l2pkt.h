#ifndef L2PKT_H
#define L2PKT_H

#pragma pack(push,1)
typedef struct _etherHdr_s {
	uint8	da[6], sa[6];
	uint16	etherType;
} _etherHdr_t;

typedef struct _vlanHdr_s {
	uint16	tag;
	uint16	etherType;
} _vlanHdr_t;


typedef struct _cpu_rx_Hdr_union1_for_pon_port_s {
#ifdef _LITTLE_ENDIAN
	uint8 pon_stream_id_h:5;
	uint8 priority:3;

	uint8 spa:4;
	uint8 org:1;
	uint8 l3r:1;
	uint8 pon_stream_id_l:2;	
#else
	uint8 priority:3;
	uint8 pon_stream_id_h:5;

	uint8 pon_stream_id_l:2;
	uint8 l3r:1;
	uint8 org:1;
	uint8 spa:4;
#endif
}_cpu_rx_Hdr_union1_for_pon_port_t;

typedef struct _cpu_rx_Hdr_union1_for_ext_port_s {
#ifdef _LITTLE_ENDIAN
	uint8 rsv_h:2;
	uint8 ext_spa:3;
	uint8 priority:3;

	uint8 spa:4;
	uint8 org:1;
	uint8 l3r:1;
	uint8 rsv_l:2;	
#else
	uint8 priority:3;
	uint8 ext_spa:3;
	uint8 rsv_h:2;

	uint8 rsv_l:2;	
	uint8 l3r:1;
	uint8 org:1;
	uint8 spa:4;
#endif
}_cpu_rx_Hdr_union1_for_ext_port_t;

typedef struct _cpu_rx_Hdr_general_for_other_port_s {
	uint16 ether_type;
	uint8 protocol;
	uint8 reason;
} _cpu_rx_Hdr_general_for_other_port_t;

typedef struct _cpu_rx_Hdr_union1_for_other_port_s {
#ifdef _LITTLE_ENDIAN

	uint8 streamid_extspa_h:5;
	uint8 priority:3;
	
	uint8 spa:4;
	uint8 org:1;
	uint8 l3r:1;
	uint8 streamid_extspa_l:2;
#else
	uint8 priority:3;
	uint8 streamid_extspa_h:5;

	uint8 streamid_extspa_l:2;
	uint8 l3r:1;
	uint8 org:1;
	uint8 spa:4;
#endif
}_cpu_rx_Hdr_union1_for_other_port_t;


typedef struct _cpu_rx_Hdr_union2_for_flow_based_s {
#ifdef _LITTLE_ENDIAN
	uint8 flow_based_hash_index_h:7;
	uint8 fbi:1;

	uint8 flow_based_hash_index_l:8;
#else
	uint8 fbi:1;
	uint8 flow_based_hash_index_h:7;
	
	uint8 flow_based_hash_index_l:8;

#endif
}_cpu_rx_Hdr_union2_for_flow_based_t;


typedef struct _cpu_rx_Hdr_union2_for_switch_s {
#ifdef _LITTLE_ENDIAN
	uint8 rsv_h:8;

	uint8 ext_port_mask:7;
	uint8 rsv_l:1;
#else
	uint8 rsv_h:8;

	uint8 rsv_l:1;
	uint8 ext_port_mask:7;
#endif
}_cpu_rx_Hdr_union2_for_switch_t;

typedef struct _cpu_rx_Hdr_s {
	uint16	ether_type;
	uint8 protocal;
	uint8 reason;
	union{
		_cpu_rx_Hdr_union1_for_pon_port_t pon_field;
		_cpu_rx_Hdr_union1_for_ext_port_t ext_field;
		_cpu_rx_Hdr_union1_for_other_port_t other_field;
	}field_union_1;

	union{
		 _cpu_rx_Hdr_union2_for_flow_based_t fb_field;
		 _cpu_rx_Hdr_union2_for_switch_t switch_field;
	}field_union_2;
}_cpu_rx_Hdr_t;




//#if defined(CONFIG_APOLLO)
typedef struct _ctag_cpu_other_header_s {
	uint16 ether_type;
	uint8 protocol;
	uint8 reason;
#ifdef _LITTLE_ENDIAN
	uint8 ttlde:5;
	uint8 priority:3;
#else
	uint8 priority:3;
	uint8 ttlde:5;
#endif
#ifdef _LITTLE_ENDIAN
	uint8 spa:3;
	uint8 resv:3;
	uint8 org:1;
	uint8 l3r:1;
#else
	uint8 l3r:1;
	uint8 org:1;
	uint8 resv:3;
	uint8 spa:3;
#endif
#ifdef _LITTLE_ENDIAN
	uint8 dslspa1:2;
	uint8 resv1:6;
#else
	uint8 resv1:6;
	uint8 dslspa1:2;
#endif
#ifdef _LITTLE_ENDIAN
	uint8 dpextmsk:6;
	uint8 dslspa2:2;
#else
	uint8 dslspa2:2;
	uint8 dpextmsk:6;
#endif
} _ctag_cpu_other_header_t;

typedef struct _ctag_cpu_dsl_header_s {
	uint16 ether_type;
	uint8 protocol;
#ifdef _LITTLE_ENDIAN
	uint8 resv1:5;
	uint8 priority:3;
#else
	uint8 priority:3;
	uint8 resv1:5;
#endif
	uint16 resv2;
	uint16 dslmsk;
} _ctag_cpu_dsl_header_t;

typedef struct _ctag_cpu_tx2_header_s {
	uint16 ether_type;
	uint8 protocol;
	uint8 txmsk_7_0;

#ifdef _LITTLE_ENDIAN
	uint8 pri:3;
	uint8 prisel:1;
	uint8 rsv0:1;
	uint8 txmsk_10_8:3;
#else
	uint8 txmsk_10_8:3;
	uint8 rsv0:1;
	uint8 prisel:1;
	uint8 pri:3;
#endif

#ifdef _LITTLE_ENDIAN
	uint8 rsv3:1;
	uint8 directTx:1;
	uint8 resv2:2;
	uint8 psel:1;
	uint8 dislrn:1;
	uint8 rsv1:1;
	uint8 keep:1;
#else
	uint8 keep:1;
	uint8 rsv1:1;
	uint8 dislrn:1;
	uint8 psel:1;
	uint8 resv2:2;
	uint8 directTx:1;
	uint8 rsv3:1;
#endif

#ifdef _LITTLE_ENDIAN
	uint16 pppoeidx_h:3;
	uint16 pppoeact:2;
	uint16 extspa:3;
	uint16 streamid_extmsk:7;
	uint16 pppoeidx_l:1;
#else
	uint16 extspa:3;
	uint16 pppoeact:2;
	uint16 pppoeidx_h:3;
	uint16 pppoeidx_l:1;
	uint16 streamid_extmsk:7;
#endif

} _ctag_cpu_tx2_header_t;

typedef struct _ctag_cpu_dsltx_header_s {
	uint16 ether_type;
	uint8 protocol;
#ifdef _LITTLE_ENDIAN
	uint8 dslspa:4;
	uint8 trap:1;
	uint8 resv:3;
#else
	uint8 resv:3;
	uint8 trap:1;
	uint8 dslspa:4;
#endif

} _ctag_cpu_dsltx_header_t;


//#endif

typedef struct _llcHdr_s {
	uint8	llc_dsap, llc_ssap, ctrl;
	uint8	org_code[3];
	uint16	etherType;
} _llcHdr_t;

typedef struct _pppoeHdr_s {
  #ifdef _LITTLE_ENDIAN
	uint8	type:4, ver:4;
  #else
	uint8	ver:4, type:4;
  #endif
	uint8	code;
	uint16	sessionId, 
			length; //Length of the PPPoE payload, does not include Ethernet and PPPoE header
	uint16	proto;	//PPP protocol field
} _pppoeHdr_t;

typedef struct _grepppHdr_s {
	uint8	adress;
	uint8	control;
	uint16	proto;	//protocol field - IPv4 or IPv6
} _grepppHdr_t;
#pragma pack(pop)


#endif

