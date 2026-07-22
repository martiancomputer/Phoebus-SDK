/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : utility Header
*
* $Author $
*
* $Revision: 1.6 $
* $Id: utility.h,v 1.6 2012/05/16 10:47:27 luke Exp $
* $Log: utility.h,v $
* Revision 1.6  2012/05/16 10:47:27  luke
* *: fix the hash function of IPv6 neighbor table entry index
*
* Revision 1.5  2012/04/27 10:56:08  ysleu
* *: For FPGA testing, fix some warrings.
*
* Revision 1.4  2012/04/27 10:51:17  luke
* *: add declaration for ipv6 utilities
*
* Revision 1.3  2012/04/26 08:39:40  ysleu
* *: Fix warrings.
*
* Revision 1.2  2012/04/24 04:05:23  ysleu
* *: Fix translator tx to extension port bugs.
*
* Revision 1.1  2012/03/15 02:27:59  ysleu
* +: Move packet generator to ./rtlTest.
*
* Revision 1.2  2012/03/06 07:25:47  ysleu
* *: Discard RTL865xC register definition.
*
* Revision 1.1.1.1  2011/12/08 11:15:45  ysleu
* +: APOLLO project initial version.
*
* Revision 1.2  2011/09/23 02:33:43  ysleu
* *: Support DSL VC and Ext. port tx with cpu tag.
*
* Revision 1.1  2011/06/08 08:21:41  ysleu
* *: Add APOLLO project model and test codes.
*
* Revision 1.4  2007/01/02 01:40:03  caroline_liu
* add generate ipv6 pkt
*
* Revision 1.3  2005/09/09 15:04:14  yjlou
* *: Fixed for Kernel Model Code. Including:
*    change printf() to rtlglue_printf()
*    unused variables
*    uninitialized variables
*    blah blah
*
* Revision 1.2  2005/06/20 13:58:47  yjlou
* *: merge PktProc
* +: add rough packet parser model code and test bench.
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.2  2004/02/17 13:01:47  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.4  2002/09/27 15:45:49  waynelee
* -> hex2bin
*
*/

#ifndef UTILITY_H
#define UTILITY_H

#include "types.h"
#include "rtl_util.h"



//void memDump(void *start, uint32 size, int8 * strHeader);
uint16 ipcsum(uint16 *ptr, uint32 len, uint16 resid);
uint16 ipadjcsum(uint16 old, uint32 diff);

void set_mac (int8 *a, int8 h1, int8 h2, int8 h3, int8 h4, int8 h5, int8 h6);
void set_v6addr(int8 *a,int8 h1,int8 h2,int8 h3,int8 h4,int8 h5,int8 h6,int8 h7,int8 h8,int8 h9,int8 h10,int8 h11,int8 h12,int8 h13,int8 h14,int8 h15,int8 h16);
void set_v6addr2 (uint8 *dest, uint32 *source);
void set_v6addr3 (uint8* dest, uint32* subnet, uint8* ifid);
void set_v6ifid(uint8 *dest, uint64 source);
uint8 v6NeighborHash(uint8 *dip, uint8 rtidx);
void set_EUI(uint8 *ifid, uint8 *mac);
void set_EUI_64(uint64 *ifid_ori, uint8 *mac);
void set_EUI(uint8 *ifid, uint8 *mac);



//Network order part
//int inet_aton(const int8 *cp, struct in_addr *addr);
/*deprecated.*/
uint32 inet_addr(const int8 *cp);
//int8 *inet_ntoa(struct in_addr ina);


#define ETHER_ADDR_LEN                          6

struct  ether_addr {
        uint8 octet[ETHER_ADDR_LEN];
};

/*
 * Convert an ASCII representation of an ethernet address to
 * binary form.
 */
struct ether_addr * ether_aton (const int8 *a);

/*
 * Convert a binary representation of an ethernet address to
 * an ASCII string.
 */
int8 * ether_ntoa (const struct ether_addr *n);

#if 0 // compiler error in linux3.18
int8 * hex2bin (const int8 *a);
#endif

#ifdef RTL865X_MODEL_USER
/* The following information should be sync to man page. */
#ifndef htonl
uint32 htonl(uint32 hostlong);
#endif
#ifndef htons
uint16 htons(uint16 hostshort);
#endif
#ifndef ntohl
uint32 ntohl(uint32 netlong);
#endif
#ifndef ntohs
uint16 ntohs(uint16 netshort);
#endif
#endif


/*===================================================================
 *  ONE's COMPLEMENT OPERATION
 *  All the operands and return value are 'host-order'.
 *  Please notice the endian problem.
 *===================================================================*/
/* One's Complement ADD */
inline static uint16 OCADD( uint16 a, uint16 b )
{
	uint32 t; /* temp */
	t = ((a)&0xffff)+((b)&0xffff);
	t = (t&0xffff)+(t>>16);
	return (uint16)t;
}

/* One's Complement NEGtive */
inline static uint16 OCNEG( uint16 a )
{
	return (~a)&0xffff;
}

/* One's Complement SUBtract */
inline static uint16 OCSUB( uint16 a, uint16 b )
{
	return OCADD( a, OCNEG(b) );
}

#endif
