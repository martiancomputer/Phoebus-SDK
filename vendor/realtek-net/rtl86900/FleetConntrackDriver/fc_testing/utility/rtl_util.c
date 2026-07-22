#include <rtl_types.h>
#include <rtl_glue.h>
#include <rtl_util.h>
#include "types.h"

#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/string.h>
#endif

#define IN6PTON_XDIGIT          0x00010000
#define IN6PTON_DIGIT           0x00020000
#define IN6PTON_COLON_MASK      0x00700000
#define IN6PTON_COLON_1         0x00100000      /* single : requested */
#define IN6PTON_COLON_2         0x00200000      /* second : requested */
#define IN6PTON_COLON_1_2       0x00400000      /* :: requested */
#define IN6PTON_DOT             0x00800000      /* . */
#define IN6PTON_DELIM           0x10000000
#define IN6PTON_NULL            0x20000000      /* first/tail */
#define IN6PTON_UNKNOWN         0x40000000

static inline int xdigit2bin(char c, int delim)
{
     if (c == delim || c == '\0')
             return IN6PTON_DELIM;
     if (c == ':')
             return IN6PTON_COLON_MASK;
     if (c == '.')
             return IN6PTON_DOT;
     if (c >= '0' && c <= '9')
             return (IN6PTON_XDIGIT | IN6PTON_DIGIT| (c - '0'));
     if (c >= 'a' && c <= 'f')
             return (IN6PTON_XDIGIT | (c - 'a' + 10));
     if (c >= 'A' && c <= 'F')
             return (IN6PTON_XDIGIT | (c - 'A' + 10));
     if (delim == -1)
             return IN6PTON_DELIM;
     return IN6PTON_UNKNOWN;
}


/*
 * Convert an ASCII string to a
 * binary representation of mac address
*/
int32 strToMac(uint8 *pMac, int8 *pStr)
{
	int8 *ptr;
	uint32 k;

	ASSERT (pMac != NULL);
	ASSERT (pStr != NULL);

	memset(pMac, 0,sizeof(ether_addr_t));
	ptr = pStr;

	for ( k = 0 ; *ptr ; ptr ++ )
	{
		if (*ptr == ' ')
		{
		} else if ( (*ptr == ':') || (*ptr == '-') )
		{
			k ++;
		} else if ( ('0' <= *ptr) && (*ptr <= '9') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'0');
		} else if ( ('a' <= *ptr) && (*ptr <= 'f') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'a'+10);
		} else if( ('A' <= *ptr) && (*ptr <= 'F') )
		{
			pMac[k] = (pMac[k]<<4) + (*ptr-'A'+10);
		} else
		{
			break;
		}
	}

	if (k != 5)
	{
		return -1;
	}

	return 0;
}


void macstrtou64(char *gatewayMac, uint64 *value){
	rtk_mac_t ethaddr;
	
	strtomac(&ethaddr, gatewayMac);
	*value = ((uint64)ethaddr.octet[0]<<40) | ((uint64)ethaddr.octet[1]<<32) | ((uint64)ethaddr.octet[2]<<24) | 
		((uint64)ethaddr.octet[3]<<16) | ((uint64)ethaddr.octet[4]<<8) | ((uint64)ethaddr.octet[5]);
	//rtlglue_printf("ethaddr 0x%llx = %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\r\n", *value, ethaddr.octet[0], ethaddr.octet[1], ethaddr.octet[2], ethaddr.octet[3], ethaddr.octet[4], ethaddr.octet[5]);

}

void mactou64(rtk_mac_t *ethaddr, uint64 *value){
	*value = ((uint64)ethaddr->octet[0]<<40) | ((uint64)ethaddr->octet[1]<<32) | ((uint64)ethaddr->octet[2]<<24) | 
		((uint64)ethaddr->octet[3]<<16) | ((uint64)ethaddr->octet[4]<<8) | ((uint64)ethaddr->octet[5]);
}

void u64tomac(uint64 value, rtk_mac_t *ethaddr)
{
	ethaddr->octet[0] = (value>>40)&0xff;
	ethaddr->octet[1] = (value>>32)&0xff;
	ethaddr->octet[2] = (value>>24)&0xff;
	ethaddr->octet[3] = (value>>16)&0xff;
	ethaddr->octet[4] = (value>>8)&0xff;
	ethaddr->octet[5] = (value>>0)&0xff;
}

uint8* strtomac(rtk_mac_t *mac, int8 *str)
{
	strToMac((uint8*)mac, str);
	return (uint8*)mac;
}

void mactostr(rtk_mac_t *mac, int8 *str)
{
	int i = 0;
	for (i = 0; i < ETHER_ADDR_LEN-1; i++, str += 3)
		sprintf(str, "%02X:", mac->octet[i]);
	sprintf(str, "%02X", mac->octet[i]);
}

void u64tomacstr(uint64 value, int8 *str)
{
	int i = 0;
	for (i = 0; i < ETHER_ADDR_LEN-1; i++, str += 3)
		sprintf(str, "%02X:", (uint8)((value>>(40-i*8))&0xff));
	sprintf(str, "%02X", (uint8)(value&0xff));
}

uint32  strtoip(ipaddr_t *ip, int8 *str)
{
    int32 t1, t2, t3, t4;
    sscanf((char*)str, "%d.%d.%d.%d", &t1, &t2, &t3, &t4);
    *ip = (t1<<24)|(t2<<16)|(t3<<8)|t4;
	return *(uint32*)ip;
}

uint32 bitNumbers(uint32 bitmap)
{
	int i = 0, count = 0;
	for(i = 0; i<32; i++)
	{
		if(bitmap & (1<<i)) count++;
	}
	//rtlglue_printf("## count = %d\r\n", count);
	return count;
}




