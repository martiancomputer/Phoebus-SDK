#ifndef _RTL_UTIL_
#define _RTL_UTIL_

#include <common/type.h>
#include <rtl_types.h>

#if 0
void IntToAscii(int32 n, int8 s[]);
#endif

int in4_pton(const char *src, int srclen,
             uint8 *dst,
             int delim, const char **end);
int in6_pton(const char *src, int srclen,
	     uint8 *dst,
	     int delim,const char *end);
	     
uint32  strtoip(ipaddr_t *ip, int8 *str);
int32 strToMac(uint8 *pMac, int8 *pStr);
uint8* strtomac(rtk_mac_t *mac, int8 *str);
void macstrtou64(char *gatewayMac, uint64 *value);
void mactou64(rtk_mac_t *ethaddr, uint64 *value);
uint32 bitNumbers(uint32 bitmap);


#endif

